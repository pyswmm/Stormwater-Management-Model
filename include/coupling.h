// --- use "C" linkage for C++ programs

#ifdef __cplusplus
extern "C" { 
#endif 

int opening_findCouplingType(double crestElev, double nodeHead, double overlandHead, double overflowArea, double weirWidth);

#ifdef __cplusplus 
}   // matches the linkage specification from above */ 
#endif
