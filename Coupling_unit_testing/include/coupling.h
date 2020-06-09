// --- use "C" linkage for C++ programs

#ifdef __cplusplus
extern "C" { 
#endif 

void coupling_adjustInflows(TCoverOpening* opening, double inflowAdjustingFactor);
int opening_findCouplingType(double crestElev, double nodeHead, double overlandHead, double overflowArea, double weirWidth);
double opening_findCouplingInflow(int couplingType, double crestElev,
                                double nodeHead, double overlandHead, double orificeCoeff, 
                                double freeWeirCoeff, double subWeirCoeff, double overflowArea, 
                                double weirWidth);
double coupling_findNodeInflow(double tStep, double Node_invertElev, double Node_fullDepth, 
                               double Node_newDepth, double Node_overlandDepth, 
                               TCoverOpening * opening, double Node_couplingArea);
int coupling_setOpening(int j, int idx, int oType, double A, double l, double Co, double Cfw, double Csw);
void coupling_execute(double tStep);

#ifdef __cplusplus 
}   // matches the linkage specification from above */ 
#endif
