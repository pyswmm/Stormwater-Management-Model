
int DLLEXPORT swmm_getStorageParam(int index, int Param, double *value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_StorageProperty)
// Output:  value = value to be output
// Return:  API Error
// Purpose: Gets Storage Parameter
{
    int errcode = 0;
    *value = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[Storage])
      {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
			 case SM_STORAGECOEFFICIENT:
                *value = Storage[index].aCoeff * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_STORAGEEXFIL:
                *value = Storage[index].exfil * UCF(LENGTH); break;
            case SM_STORAGEEVAP:
                *value = Storage[index].fEvap * UCF(EVAPRATE); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
	}
    return(errcode);
}

// Set Storage Parameters

int DLLEXPORT swmm_setStorageParam(int index, int Param, double value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_StorageProperty)
//          value = value to be input
// Return:  API Error
// Purpose: Sets Storage Parameter
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
     // Check if Simulation is Running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        errcode = ERR_API_SIM_NRUNNING;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[STORAGE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
            case SM_STORAGECOEFFICIENT:
                *value = Storage[index].aCoeff * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_STORAGEEXFIL:
                *value = Storage[index].exfil * UCF(LENGTH); break;
            case SM_STORAGEEVAP:
                *value = Storage[index].fEvap * UCF(EVAPRATE); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    // Re-validated a storage BEM 1/20/2017 Probably need to re-validate connecting links
    //storage_validate(index)
    return(errcode);
}

