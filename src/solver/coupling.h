// --- use "C" linkage for C++ programs

#ifdef __cplusplus
extern "C" { 
#endif 

int opening_findCouplingType(double crestElev, double nodeHead, double overlandHead, double overflowArea, double weirWidth);
double opening_findCouplingInflow(int couplingType, double crestElev,
                                double nodeHead, double overlandHead, double orificeCoeff, 
                                double freeWeirCoeff, double subWeirCoeff, double overflowArea, 
                                double weirWidth);
int coupling_setOpening(int j, int idx, int oType, double A, double l, double Co, double Cfw, 
                        double Csw);
int coupling_countOpenings(int j);
int coupling_deleteOpening(int j, int idx);
double coupling_findNodeInflow(int j, double tStep, double Node_invertElev, double Node_fullDepth, double Node_newDepth, double Node_overlandDepth, 
							   double Node_couplingArea);
void coupling_adjustInflows(TCoverOpening* opening, double inflowAdjustingFactor);
int coupling_openOpening(int j, int idx);
int coupling_closeOpening(int j, int idx);
void coupling_deleteOpenings(int j);

#ifdef __cplusplus 
}   // matches the linkage specification from above */ 
#endif
