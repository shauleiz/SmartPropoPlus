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
char * GetCurrentMixerDeviceFromRegistry();
void SetCurrentMixerDeviceToRegistry(const char * MixerName);
int GetCurrentInputLineFromRegistry(unsigned int *SrcID);
void SetCurrentInputLineToRegistry(const char * MixerName, unsigned int SrcID);
int SetActiveModeToRegistry(const char * selected);
void SetShiftAutoDetectToRegistry(const int sel);
void SetPositiveShiftToRegistry(const int sel);
struct Modulations * GetModulationFromRegistry(int Create);

int CreateDefaultSppRegistry();
int CreateEmptyFmsRegistry();

int isAudioRegistryExist();
int isSppRegistryExist();
int isFmsRegistryExist();

#if defined(__cplusplus)
}
#endif
