/** SmartPropoPlus Registry Interface **/
#include "windows.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/* Function prototypes */
int GetDefaultVolumeValueFromRegistry(unsigned long SrcType, unsigned long * VolumeValue);
int SetDefaultVolumeValueToRegistry(unsigned long SrcType, unsigned long  VolumeValue);
int GetCurrentAudioStateFromRegistry();
void SetCurrentAudioStateToRegistry(int Active);
char * GetCurrentMixerDeviceFromRegistry(void);
void SetCurrentMixerDeviceToRegistry(const char * MixerName);
int GetCurrentInputLineFromRegistry(unsigned int *SrcID);
void SetCurrentInputLineToRegistry(const char * MixerName, unsigned int SrcID);
int SetActiveModeToRegistry(const char * selected);
int GetWaveInParamsFromRegistry(int *nSamples, int *nBits, int *nBuffers, int *BufferSize);
void SetShiftAutoDetectToRegistry(const int sel);
void SetPositiveShiftToRegistry(const int sel);
struct Modulations * GetModulationFromRegistry(int Create);
int GetDebugLevelFromRegistry(void);
int GetCurrentPpjoyStateFromRegistry();
void SetCurrentPpjoyStateToRegistry(int Active);

int CreateDefaultSppRegistry();
int CreateDefaultModRegistry();
int UpdateModRegistry();
int CreateEmptyBaseRegistry();

int isAudioRegistryExist();
int isSppRegistryExist();
int isBaseRegistryExist();
int isModRegistryUpdate();

int SetSelectedFilterNameToRegistry(const char * selected);
char * GetSelectedFilterNameFromRegistry();

BOOL GetLocationDialogWindowFromRegistry(int * x, int * y);
void SetLocationDialogWindowToRegistry(int x,int y);

int GetAntiJitterStateFromRegistry(void);
void SetAntiJitterStateToRegistry(int enable);

int GetSppConsoleFullPathFromRegistry(char * Path, UINT PathSize);
int GetFmsFullPathFromRegistry(char * Path, UINT PathSize);

#if defined(__cplusplus)
}
#endif
