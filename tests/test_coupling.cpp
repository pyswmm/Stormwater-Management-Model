/*
 *   test_coupling.cpp
 *
 *   Created: 09/06/2020
 *   Author: Velitchko G. Tzatchkov
 *           Laurent Courty
 *           IMTA, Mexico
 *
 *   Unit testing for SWMM coupling using Boost Test.
 */

#define BOOST_TEST_MODULE "coupling"
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
//#include "swmm5.h"
//#include "toolkitAPI.h"
//#include "enums.h"
//#include "error.h"
//#include "consts.h"
//#include "objects.h"
#include "test_toolkitapi_coupling.hpp"

struct CoverOpening
{
   int            ID;              // opening index number
   int            type;            // type of opening (grate, etc). From an enum
   int            couplingType;    // type of surface coupling (enum SurfaceCouplingType)
   double         area;            // area of the opening (ft2)
   double         length;          // length of the opening (~circumference, ft)
   double         coeffOrifice;    // orifice coefficient
   double         coeffFreeWeir;   // free weir coefficient
   double         coeffSubWeir;    // submerged weir coefficient
   double         oldInflow;       // inflow during last time-step
   double         newInflow;       // current inflow
   struct CoverOpening* next;      // pointer to next opening data object
};
typedef struct CoverOpening TCoverOpening;

struct ExtInflow
{
   int            param;         // pollutant index (flow = -1)
   int            type;          // CONCEN or MASS
   int            tSeries;       // index of inflow time series
   int            basePat;       // baseline time pattern
   double         cFactor;       // units conversion factor for mass inflow
   double         baseline;      // constant baseline value
   double         sFactor;       // time series scaling factor
   double         extIfaceInflow;// external interfacing inflow
   struct ExtInflow* next;       // pointer to next inflow data object
};
typedef struct ExtInflow TExtInflow;

struct DwfInflow
{
   int            param;          // pollutant index (flow = -1)
   double         avgValue;       // average value (cfs or concen.)
   int            patterns[4];    // monthly, daily, hourly, weekend time patterns
   struct DwfInflow* next;        // pointer to next inflow data object
};
typedef struct DwfInflow TDwfInflow;

typedef struct
{
   int           unitHyd;         // index of unit hydrograph
   double        area;            // area of sewershed (ft2)
}  TRdiiInflow;

//  Node in a tokenized math expression list
struct ExprNode
{
    int    opcode;                // operator code
    int    ivar;                  // variable index
    double fvalue;                // numerical value
	struct ExprNode *prev;        // previous node
    struct ExprNode *next;        // next node
};
typedef struct ExprNode MathExpr;

typedef struct
{
    int          treatType;       // treatment equation type: REMOVAL/CONCEN
    MathExpr*    equation;        // treatment eqn. as tokenized math terms
} TTreatment;

typedef struct
{
   char*         ID;              // node ID
   int           type;            // node type code
   int           subIndex;        // index of node's sub-category
   char          rptFlag;         // reporting flag
   double        invertElev;      // invert elevation (ft)
   double        initDepth;       // initial storage level (ft)
   double        fullDepth;       // dist. from invert to surface (ft)
   double        surDepth;        // added depth under surcharge (ft)
   double        pondedArea;      // area filled by ponded water (ft2)
   double        surfaceArea;     // area used to calculate node's volume (ft2)
   TExtInflow*   extInflow;       // pointer to external inflow data
   TDwfInflow*   dwfInflow;       // pointer to dry weather flow inflow data
   TRdiiInflow*  rdiiInflow;      // pointer to RDII inflow data
   TTreatment*   treatment;       // array of treatment data
   //-----------------------------
   TCoverOpening* coverOpening;   // pointer to node opening data
   double        couplingArea;    // coupling area in the overland model (ft2)
   double        overlandDepth;   // water depth in the overland model (ft)
   double        couplingInflow;  // flow from the overland model (cfs)
   //-----------------------------
   int           degree;          // number of outflow links
   char          updated;         // true if state has been updated
   double        crownElev;       // top of highest flowing closed conduit (ft)
   double        inflow;          // total inflow (cfs)
   double        outflow;         // total outflow (cfs)
   double        losses;          // evap + exfiltration loss (ft3)
   double        oldVolume;       // previous volume (ft3)
   double        newVolume;       // current volume (ft3)
   double        fullVolume;      // max. storage available (ft3)
   double        overflow;        // overflow rate (cfs)
   double        oldDepth;        // previous water depth (ft)
   double        newDepth;        // current water depth (ft)
   double        oldLatFlow;      // previous lateral inflow (cfs)
   double        newLatFlow;      // current lateral inflow (cfs)
   double*       oldQual;         // previous quality state
   double*       newQual;         // current quality state
   double        oldFlowInflow;   // previous flow inflow
   double        oldNetInflow;    // previous net inflow
}  TNode;

enum  OverlandCouplingType {
      NO_COUPLING,
      NO_COUPLING_FLOW,
      ORIFICE_COUPLING,
      FREE_WEIR_COUPLING,
      SUBMERGED_WEIR_COUPLING};

#define   GRAVITY            32.2           // accel. of gravity in US units
#define MAX_OBJ_TYPES 16
#define ERR_NONE 0
#define ERR_API_OBJECT_INDEX 505
#define ERR_MEMORY 101

// The following two declarations are required by coupling.c to work
// TNode* Node = new TNode[2];
TNode* Node[2];
//Node[0]->coveropening = NULL;
//Node[1]->coveropening = NULL;
int Nobjects[MAX_OBJ_TYPES];

//#include "coupling.h"

int opening_findCouplingType(double crestElev, double nodeHead, double overlandHead, 
                             double overflowArea, double weirWidth)
//
//  Input:   crestElev = elevation of the node crest (= ground) (ft)
//           nodeHead = water elevation in the node (ft)
//           overlandHead = water elevation in the overland model (ft)
//           overflowArea = node surface area (ft2)
//           weirWidth = weir width (ft)
//  Output:  Type of Coupling
//  Purpose: determine the coupling type of an opening
//           according the the relative water elevations in node and surface
//
{
    int overflow, drainage;
    int overflowOrifice, drainageOrifice, submergedWeir, freeWeir;
    double surfaceDepth, weirRatio;
 
    surfaceDepth = overlandHead - crestElev;
    weirRatio = overflowArea / weirWidth;

    // --- boolean cases. See DOI 10.1016/j.jhydrol.2017.06.024
    overflow = nodeHead > overlandHead;
    drainage = nodeHead < overlandHead;
    overflowOrifice = overflow && (nodeHead > crestElev);    
    drainageOrifice = drainage &&
                        ((nodeHead > crestElev) && (surfaceDepth >= weirRatio));
    submergedWeir = drainage &&
                    ((nodeHead > crestElev) && (surfaceDepth < weirRatio));
    freeWeir = drainage && (nodeHead < crestElev) && (overlandHead > crestElev);

    // --- set the coupling type
    if ( !overflow && !drainage ) return NO_COUPLING_FLOW;
    else if ( overflowOrifice || drainageOrifice ) return ORIFICE_COUPLING;
    else if ( submergedWeir ) return SUBMERGED_WEIR_COUPLING;
    else if ( freeWeir ) return FREE_WEIR_COUPLING;
    else return NO_COUPLING_FLOW;
}

double opening_findCouplingInflow(int couplingType, double crestElev,
                                double nodeHead, double overlandHead, double orificeCoeff, 
                                double freeWeirCoeff, double subWeirCoeff, double overflowArea, 
                                double weirWidth)
//
//  Input:   couplingType = Type of Coupling
//           nodeHead = water elevation in the node (ft)
//           crestElev = elevation of the node crest (= ground) (ft)
//           overlandHead = water elevation in the overland model (ft)
//           orificeCoeff = orifice coefficient
//           freeWeirCoeff = free weir coefficient
//           overflowArea = node surface area (ft2)
//           weirWidth = weir width (ft)
//  Output:  the flow entering through the opening (ft3/s)
//  Purpose: computes the coupling flow of the opening
//
{
    double headUp, headDown, headDiff, depthUp;
    double orif_v, sweir_v, u_couplingFlow;

	double exponente= 1.5;
	double sqrt2g = sqrt(2 * GRAVITY);

    headUp = fmax(overlandHead, nodeHead);
    headDown = fmin(overlandHead, nodeHead);
    headDiff = headUp - headDown;
    depthUp =  headUp - crestElev;

    switch(couplingType)
    {
        case ORIFICE_COUPLING:
            orif_v = sqrt(2 * GRAVITY * headDiff);
            u_couplingFlow = orificeCoeff * overflowArea * orif_v;
            break;
        case FREE_WEIR_COUPLING:
            u_couplingFlow = (2/3.) * freeWeirCoeff * weirWidth *
                             pow(depthUp, exponente) * sqrt2g;
            break;
        case SUBMERGED_WEIR_COUPLING:
            sweir_v = sqrt(2. * GRAVITY * headDiff);
            u_couplingFlow = subWeirCoeff * weirWidth * depthUp * sweir_v;
            break;
        default: u_couplingFlow = 0.0; break;
    }
    // --- Flow into the node is positive
    // opening->newInflow = copysign(u_couplingFlow, overlandHead - nodeHead);
	if (overlandHead - nodeHead > 0)
	{
		// opening->newInflow = fabs(u_couplingFlow);
        return fabs(u_couplingFlow);
	} else
	{
		// opening->newInflow = -fabs(u_couplingFlow);
		return  -fabs(u_couplingFlow);
	} 
}

//=============================================================================
void coupling_adjustInflows(TCoverOpening* opening, double inflowAdjustingFactor)
//
//  Input:   opening = pointer to node opening data
//           inflowAdjustingFactor = an inflow adjusting coefficient
//  Output:  none
//  Purpose: adjust the inflow according to an adjusting factor
//
{
    // --- iterate among the openings
    while ( opening )
    {
        opening->newInflow = opening->newInflow * inflowAdjustingFactor;
        opening = opening->next;
    }
}

//=============================================================================

double coupling_findNodeInflow(double tStep, double Node_invertElev, double Node_fullDepth, double Node_newDepth, double Node_overlandDepth, 
							   TCoverOpening * opening, double Node_couplingArea)
//
//  Input:   tStep = time step of the drainage model (s)
//           Node_invertElev = invert elevation (ft)
//           Node_fullDepth = dist. from invert to surface (ft)
//           Node_newDepth = current water depth (ft)
//           overlandDepth = water depth in the overland model (ft)
//           opening = pointer to node opening data
//           Node_couplingArea = coupling area in the overland model (ft2)
//  Output:  coupling inflow
//  Purpose: compute the coupling inflow for each node's opening].overlandDepth
//
{
    double crestElev, overlandHead, nodeHead;
    double totalCouplingInflow;
    double rawMaxInflow, maxInflow, inflowAdjustingFactor;
    int inflow2outflow, outflow2inflow;
	TCoverOpening* First_opening = opening;

    // --- calculate elevations
    crestElev = Node_invertElev + Node_fullDepth;
    nodeHead = Node_invertElev + Node_newDepth;
    overlandHead = crestElev + Node_overlandDepth;

    // --- init
    totalCouplingInflow = 0.0;

    // --- iterate among the openings
    while ( opening )
    {
        // --- do nothing if not coupled
        if ( opening->couplingType == NO_COUPLING) 
        {
            opening = opening->next;
            continue;
        }
        // --- compute types and inflows
        opening->couplingType = opening_findCouplingType(crestElev, nodeHead, overlandHead, opening->area, opening->length);
        opening->newInflow = opening_findCouplingInflow(opening->couplingType, crestElev, nodeHead, overlandHead,
                                                        opening->coeffOrifice, opening->coeffFreeWeir, 
                                                        opening->coeffSubWeir, opening->area, 
                                                        opening->length);
       // --- prevent oscillations
        inflow2outflow = (opening->oldInflow > 0.0) && (opening->newInflow < 0.0);
        outflow2inflow = (opening->oldInflow < 0.0) && (opening->newInflow > 0.0);
        if (inflow2outflow || outflow2inflow)
        {
            opening->couplingType = NO_COUPLING_FLOW;
            opening->newInflow = 0.0;
        }
        // --- add inflow to total inflow
        totalCouplingInflow += opening->newInflow;
        opening = opening->next;
    }
    // --- inflow cannot drain the overland model
    if (totalCouplingInflow > 0.0)
    {
        rawMaxInflow = (Node_overlandDepth * Node_couplingArea) / tStep;
        maxInflow = fmin(rawMaxInflow, totalCouplingInflow);
        inflowAdjustingFactor = maxInflow / totalCouplingInflow;
        coupling_adjustInflows(First_opening, inflowAdjustingFactor);

        // --- get adjusted inflows
        while ( opening )
        {
            totalCouplingInflow += opening->newInflow;
            opening = opening->next;
        }
    }
    return totalCouplingInflow;
}

//=============================================================================

int coupling_closeOpening(int j, int idx)
//
//  Input:   j = node index
//           idx = opening index
//  Output:  Error code
//  Purpose: Close an opening
//
{
    int errcode = 0;
    TCoverOpening* opening;  // opening object

    // --- Find the opening
    opening = Node[j]->coverOpening;

    while ( opening )
    {
        if ( opening->ID == idx ) break;
        opening = opening->next;
    }

    // --- if opening doesn't exist, return an error
    if ( opening == NULL )
    {
        return(ERR_API_OBJECT_INDEX);
    }

    // --- Close the opening
    opening->couplingType  = NO_COUPLING;
    return 0;

}

//=============================================================================

int coupling_openOpening(int j, int idx)
//
//  Input:   j = node index
//           idx = opening index
//  Output:  Error code
//  Purpose: Open an opening
//
{
    int errcode = 0;
    TCoverOpening* opening;  // opening object

    // --- Find the opening
    opening = Node[j]->coverOpening;
    while ( opening )
    {
        if ( opening->ID == idx ) break;
        opening = opening->next;
    }
    // --- if opening doesn't exist, return an error
    if ( opening == NULL )
    {
        return(ERR_API_OBJECT_INDEX);
    }

    // --- Open the opening
    opening->couplingType  = NO_COUPLING_FLOW;
    return 0;
}

//=============================================================================

int coupling_setOpening(int j, int idx, int oType, double A, double l, double Co, double Cfw, 
                        double Csw)
// Purpose: Assigns property values to the node opening object
// Inputs:  j     = Node index
//          idx   =  opening's index
//          otype =  type of opening (grate, etc). From an enum
//          A     = area of the opening (ft2)
//          l     = length of the opening (~circumference, ft)
//          Co    = orifice coefficient
//          Cfw   = free weir coefficient
//          Csw   = submerged weir coefficient
// Return:  error code
{
    int errcode = 0;
    TCoverOpening* opening;  // opening object

    // --- check if an opening object with this index already exists
    opening = Node[j]->coverOpening;
    while ( opening )
    {
        if ( opening->ID == idx ) break;
        opening = opening->next;
    }

    // --- if it doesn't exist, then create it
    if ( opening == NULL )
    {
        opening = (TCoverOpening *) malloc(sizeof(TCoverOpening));
        if ( opening == NULL )
        {
            //return error_setInpError(ERR_MEMORY, "");
            return 101;
        }
        opening->next = Node[j]->coverOpening;
        Node[j]->coverOpening = opening;
    }

    // Assign values to the opening object
    opening->ID            = idx;
    opening->type          = oType;
    opening->area          = A;
    opening->length        = l;
    opening->coeffOrifice  = Co;
    opening->coeffFreeWeir = Cfw;
    opening->coeffSubWeir  = Csw;
    // --- default values
    opening->couplingType  = NO_COUPLING_FLOW;
    opening->oldInflow     = 0.0;
    opening->newInflow     = 0.0;
    return(errcode);
}

//=============================================================================

int coupling_countOpenings(int j)
//
//  Input:   j = node index
//  Output:  number of openings in the node
//  Purpose: count the number of openings a node have.
//
{
    int openingCounter = 0;
    TCoverOpening* opening;

    opening = Node[j]->coverOpening;
    while ( opening )
    {
        if ( opening ) openingCounter++;
        opening = opening->next;
    }
    return(openingCounter);
}

//=============================================================================

int coupling_deleteOpening(int j, int idx)
//
//  Input:   j = node index
//           idx = opening index
//  Output:  none
//  Purpose: delete an opening from a node.
//
{
    // If position to deleete is negative 
    if (idx < 0) return -1; //Bad idx - no opening to delete
    TCoverOpening* opening;
    opening = Node[j]->coverOpening;
    if (opening == NULL) return 0; //No opening to delete 
    // Store first opening
    TCoverOpening* temp = opening; 
    // If the first opening needs to be deleted 
    if (idx == 0) 
    { 
        Node[j]->coverOpening = temp->next;   // Change pointer to the second opening
        free(temp);                          // free former first opening
        return 1; 
    } 

    // Find previous opening of the opening to be deleted 
    for (int i=0; temp!=NULL && i<idx-1; i++) 
         temp = temp->next; 
  
    // Position is more than number of openings 
    if (temp == NULL || temp->next == NULL) return -2; //Bad idx - no opening to delete
  
    // Opening temp->next is the opening to be deleted 
    // Store pointer to the next of opening to be deleted 
    TCoverOpening* next = temp->next->next; 
  
    // Unlink the opening from the linked list 
    free(temp->next);  // Free memory 
  
    temp->next = next;  // Unlink the deleted node from list 
    return idx+1;
}

//=============================================================================

void coupling_deleteOpenings(int j)
//
//  Input:   j = node index
//  Output:  none
//  Purpose: deletes all opening data for a node.
//
{
    TCoverOpening* opening1;
    TCoverOpening* opening2;
    opening1 = Node[j]->coverOpening;
    while ( opening1 )
    {
        opening2 = opening1->next;
        free(opening1);
        opening1 = opening2;
    }
    Node[j]->coverOpening = NULL;
}

//=============================================================================

int couplingType;
double CouplingInflow;
double orificeCoeff = 0.167;
double freeWeirCoeff = 0.54;
double subWeirCoeff = 0.056;
double tStep = 2.0;
double Node_couplingArea= 1.0 / (.3048 * .3048);

// Test case               0     1     2     3     4     5     6     7     8     9    10
double crestElev[11] =    {2.0,  2.0,  2.0,  2.0,  2.0,  3.0,  2.0,  2.0,  2.0,  2.0, 2.0};
double nodeHead[11] =     {0.0,  2.0,  3.0,  4.0,  1.0,  2.0,  1.0,  3.0,  3.0,  3.0, 1.0};
double overlandHead[11] = {2.0,  2.0,  2.0,  3.0,  1.0,  1.0,  3.0,  4.0,  3.5,  5.0, 2.0};
double overflowArea[11] = {25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 10.0, 10.0, 10.0, 10.0, 10.0};
double weirWidth[11] =    {1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  5.0,  5.0,  5.0,  5.0, 5.0};
int ExpectedType[11] = {NO_COUPLING_FLOW, NO_COUPLING_FLOW, ORIFICE_COUPLING, ORIFICE_COUPLING,
                        NO_COUPLING_FLOW, NO_COUPLING_FLOW, FREE_WEIR_COUPLING, ORIFICE_COUPLING,
                        SUBMERGED_WEIR_COUPLING, ORIFICE_COUPLING, NO_COUPLING_FLOW};
double ExpectedQ[11] =    {0.0,  0.0,  -18.49294,  
                                            -18.49294,
                                                    0.0,  0.0,  
                                                                7.973,  
                                                                     7.39717,  
                                                                           1.31547,  
                                                                                  10.4611, 
                                                                                       0.0};
        
BOOST_AUTO_TEST_SUITE(test_coupling)

    BOOST_DATA_TEST_CASE(ParameterizedCouplingInflowTest, boost::unit_test::data::xrange(0, 11), var)
    {

        // Unit conversion - from m to ft
        crestElev[var] = crestElev[var] / .3048;       
        nodeHead[var] = nodeHead[var] / .3048;       
        overlandHead[var] = overlandHead[var] / .3048;
        weirWidth[var] = weirWidth[var] / .3048;
        overflowArea[var] = overflowArea[var] / (.3048 * .3048);

        couplingType = opening_findCouplingType(crestElev[var], nodeHead[var], overlandHead[var],
                                                overflowArea[var], weirWidth[var]);
        BOOST_CHECK_EQUAL(couplingType, ExpectedType[var]);

        CouplingInflow = opening_findCouplingInflow(couplingType, crestElev[var], nodeHead[var], overlandHead[var],
                                                    orificeCoeff, freeWeirCoeff, subWeirCoeff,
                                                    overflowArea[var], weirWidth[var]);
        CouplingInflow = CouplingInflow * (.3048 * .3048 * .3048);
        BOOST_CHECK_SMALL(abs(CouplingInflow) - abs(ExpectedQ[var]), 0.005);
        BOOST_CHECK_SMALL(CouplingInflow - ExpectedQ[var], 0.005);

    }

    BOOST_FIXTURE_TEST_CASE(NodeOpeningTests, FixtureOpenClose) 
    {
        int error, node_ind, no_of_deleted_openings, no_of_openings;
        double area, width;
        double NodeInflow;
        char node_id[] = "J0";
        error = swmm_getObjectIndex(SM_NODE, node_id, &node_ind);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(node_ind == 0);

        // No openings added
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 0);
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                                Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        BOOST_CHECK_EQUAL(NodeInflow, 0.0);

        // Add a first opening to Node[node_ind]
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        BOOST_CHECK (Node[node_ind]->coverOpening != NULL);
        // One opening added
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 1);

        // Test case 2 with one opening
        Node[node_ind]->fullDepth = 2.0 / .3048;
        Node[node_ind]->newDepth = 3.0  / .3048;
        Node[node_ind]->overlandDepth = 0.0 / .3048;
        Node[node_ind]->couplingArea = 1.0 / (.3048 * .3048);
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[2], 0.005);
        // Test case 4 with one opening
        Node[node_ind]->newDepth = 1.0  / .3048;
        Node[node_ind]->overlandDepth = -1.0 / .3048;
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is NO_COUPLING_FLOW
        BOOST_CHECK_EQUAL(NodeInflow, 0.0);
        // Test case 6 with one opening
        Node[node_ind]->newDepth = 1.0  / .3048;
        Node[node_ind]->overlandDepth = 1.0 / .3048;
        // Change area and width to correspond to test case 6
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is FREE_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[6], 0.005);
        // Test case 8 with one opening
        Node[node_ind]->newDepth = 3.0  / .3048;
        Node[node_ind]->overlandDepth = 1.5 / .3048;
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is SUBMERGED_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[8], 0.005);

        // Reset area and width of the first opening
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Add a second opening, with different area and width
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Two openings added
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 2);

        // Test case 2 with two openings
        Node[node_ind]->newDepth = 3.0  / .3048;
        Node[node_ind]->overlandDepth = 0.0 / .3048;
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[2]-7.39718), 0.01);
        // Test case 4 with two openings
        Node[node_ind]->newDepth = 1.0  / .3048;
        Node[node_ind]->overlandDepth = -1.0 / .3048;
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is NO_COUPLING_FLOW
        BOOST_CHECK_EQUAL(NodeInflow, 0.0);
        // Test case 6 with two openings
        Node[node_ind]->newDepth = 1.0  / .3048;
        Node[node_ind]->overlandDepth = 1.0 / .3048;
        // Reset area and width of the first opening
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Change area and width of the second opening, to be different test from case 6
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is FREE_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[6]+1.5946), 0.005);
        // Test case 8 with two openings
        Node[node_ind]->newDepth = 3.0  / .3048;
        Node[node_ind]->overlandDepth = 1.5 / .3048;
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is SUBMERGED_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[8]+.26310), 0.005);

        //Close the second opening
        error = coupling_closeOpening(node_ind, 1);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Test case 2 with opening 1 open and opening 2 closed
        // Reset area and width
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // There are two openings
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 2);
        Node[node_ind]->newDepth = 3.0  / .3048;
        Node[node_ind]->overlandDepth = 0.0 / .3048;
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[2], 0.005);

        // Reopen the second opening
        error = coupling_openOpening(node_ind, 1);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Test case 2 with both openings 1 and opening 2 open
        // Reset area and width of the second opening
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
    	NodeInflow = coupling_findNodeInflow(tStep, Node[node_ind]->invertElev, Node[node_ind]->fullDepth, 
                                            Node[node_ind]->newDepth, Node[node_ind]->overlandDepth, 
						                    Node[node_ind]->coverOpening, Node[node_ind]->couplingArea);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[2]-7.39718), 0.01);

        // Add a third opening to Node[node_ind]
        error = swmm_setNodeOpening(node_ind, 2, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Three openings added
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 3);
        // Delete the second opening
        no_of_deleted_openings = coupling_deleteOpening(node_ind, 1);
        // Two remaininig openings
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 2);
        // Delete first opening
        no_of_deleted_openings = coupling_deleteOpening(node_ind, 0);
        // One remaininig opening
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 1);
        // Delete the remaininig opening
        no_of_deleted_openings = coupling_deleteOpening(node_ind, 0);
        // No remaininig opening
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 0);
        // Try to delete with no remaininig opening
        no_of_deleted_openings = coupling_deleteOpening(node_ind, 0);
        BOOST_CHECK_EQUAL(no_of_deleted_openings, 0);

        // Add a first opening to Node[node_ind]
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // One opening added
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 1);
        // Add a second opening, with different area and width
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, 0.167, 0.54, 0.056);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Two openings added
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 2);
        // Delete openings
        coupling_deleteOpenings(node_ind);
        // Check if openings remain
        no_of_openings= coupling_countOpenings(node_ind);
        BOOST_CHECK_EQUAL(no_of_openings, 0);

    }

BOOST_AUTO_TEST_SUITE_END()
