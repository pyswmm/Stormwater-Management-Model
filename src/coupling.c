//-----------------------------------------------------------------------------
//   coupling.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//             09/15/14   (Build 5.1.007)
//             04/02/15   (Build 5.1.008)
//             08/05/15   (Build 5.1.010)
//             09/06/20
//   Authors:  Laurent Courty
//             Velitchko G. Tzatchkov
//
//   Copyrighted (c) distributed under the MIT license (see LICENSE.txt)
//
//   Overland model flow coupling functions.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "headers.h"

//=============================================================================

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
//           subWeirCoeff = submerged weir coefficient
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

double coupling_findNodeInflow(int j, double tStep, double Node_invertElev, double Node_fullDepth, double Node_newDepth, double Node_overlandDepth, 
							   double Node_couplingArea)
//
//  Input:   tStep = time step of the drainage model (s)
//           Node_invertElev = invert elevation (ft)
//           Node_fullDepth = dist. from invert to surface (ft)
//           Node_newDepth = current water depth (ft)
//           Node_overlandDepth = water depth in the overland model (ft)
//           opening = pointer to node opening's data
//           Node_couplingArea = coupling area in the overland model (ft2)
//  Output:  node coupling inflow
//  Purpose: compute the sum of opening coupling inflows at a node
//
{
    double crestElev, overlandHead, nodeHead;
    double totalCouplingInflow;
    double rawMaxInflow, maxInflow, inflowAdjustingFactor;
    int inflow2outflow, outflow2inflow;
	TCoverOpening* opening;
	TCoverOpening* First_opening;
    opening = Node[j].coverOpening; 
	First_opening = opening;

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

void coupling_execute(double tStep)
//
//  Input:   tStep = time step of the drainage model (s)
//  Output:  none
//  Purpose: find coupling flow for each node.
//
{
    int j;

    for ( j = 0; j < Nobjects[NODE]; j++ )
    {
        if ( !coupling_isNodeCoupled(j) ) continue;
		Node[j].couplingInflow = coupling_findNodeInflow(j, tStep, Node[j].invertElev, Node[j].fullDepth, Node[j].newDepth, Node[j].overlandDepth, 
							                             Node[j].couplingArea);    }
}

//=============================================================================

void coupling_setOldState(int j)
//
//  Input:   j = node index
//  Output:  none
//  Purpose: replaces a node's old hydraulic coupling state values with new ones.
//
{
    TCoverOpening* opening;

    // --- iterate among the openings.
    opening = Node[j].coverOpening;
    while ( opening )
    {
        opening->oldInflow = opening->newInflow;
        opening = opening->next;
    }
}

//=============================================================================

int coupling_isNodeCoupled(int j)
//
//  Input:   j = node index
//  Output:  returns the coupling status of a node (yes/no)
//  Purpose: if at least one opening is coupled, return TRUE.
//
{
    int isCoupled = FALSE;
    TCoverOpening* opening;

    // --- iterate among the openings. If one is coupled, return YES
    opening = Node[j].coverOpening;
    while ( opening )
    {
        if ( opening->couplingType != NO_COUPLING )
        {
            isCoupled = TRUE;
            break;
        }
        opening = opening->next;
    }
    return isCoupled;
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
    opening = Node[j].coverOpening;

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
    opening = Node[j].coverOpening;
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
    opening = Node[j].coverOpening;
//    while ( opening )
    while ( opening != NULL )
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
            return ERR_MEMORY;
        }
        opening->next = Node[j].coverOpening;
        Node[j].coverOpening = opening;
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

    opening = Node[j].coverOpening;
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
//  Return:  error code
//  Purpose: delete an opening from a node.
//
{
    // If position to deleete is negative 
    if (idx < 0) return -1; //Bad idx - no opening to delete
    TCoverOpening* opening;
    opening = Node[j].coverOpening;
    if (opening == NULL) return 0; //No opening to delete 
    // Store first opening
    TCoverOpening* temp = opening; 
    // If the first opening needs to be deleted 
    if (idx == 0) 
    { 
        Node[j].coverOpening = temp->next;   // Change pointer to the second opening
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
    opening1 = Node[j].coverOpening;
    while ( opening1 )
    {
        opening2 = opening1->next;
        free(opening1);
        opening1 = opening2;
    }
    Node[j].coverOpening = NULL;
}

//=============================================================================

