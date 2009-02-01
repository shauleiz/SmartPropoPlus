/* 
	Example01
	How to use JsChPostProc.Dll from 'C' code
*/
#include "Example01.h"

main()
{	 
	/*	Load the DLL & get a handle to it
		The handle will be used throughout the example to access the DLL */
	g_hFilter = load_filter_dll();
	if (!g_hFilter)		
		return -1;

	/* Test that the minimal API requiremets are met*/ 
	if (!is_inimal_api(g_hFilter))
		return -2;

	/* Display DLL version */
	print_ver(g_hFilter);

	/* Display list of filters for the user to select - get selection */
	g_selected_filter = select_filter(g_hFilter);
	if (g_selected_filter<0)
		return -3;

	/* Set the DLL to the selected filter */
	set_filter(g_hFilter, g_selected_filter);

	/* Run the filter on a single 6-channel vector */
	run_filter(g_hFilter);
	return 0;
}


/* 
	Load the filter DLL 
	The DLL file name is hard coded to "JsChPostProc.Dll"
	It is searched according the OS search algorithm - so you better just put it in the current directory
	
	Return: Handle to the DLL 
*/
HINSTANCE load_filter_dll()
{
	HINSTANCE hFilter;	/* Handler to DLL */

	/* Load the DLL and get a handle to it */ 
	hFilter = LoadLibrary("JsChPostProc.Dll");
	if (!hFilter)
	{
		fprintf(stderr, "\nError:	Cannot load file JsChPostProc.Dll\n");
		return NULL;
	};



	return hFilter;
}

/* Given the DLL handle - this function tests that the minimal API requiremets are met */
int	is_inimal_api(HINSTANCE hFilter)
{	
	char * api_function;

	/* GetDllVersion() - Get the version of the file */
	api_function = "GetDllVersion";
	pGetDllVersion = GetProcAddress(hFilter,api_function);
	if (!pGetDllVersion)
	{
		fprintf(stderr, "\nError:	API function %s was not found\n", api_function);
		return 0;
	};

	/* "ProcessChannels() - The filter itself */
	api_function = "ProcessChannels";
	pProcessChannels = (PJS_CHANNELS  (WINAPI *)(PJS_CHANNELS, int max, int min))GetProcAddress(hFilter,api_function);
	if (!pProcessChannels)
	{
		fprintf(stderr, "\nError:	API function %s was not found\n", api_function);
		return 0;
	};

	return 1;
}

/*
	If there are more that one filter
	Display the names of the filters
	Ask the user to select one

	Return:
	Zero-based index if valid selection
	Invalid selection: (-1)
*/
int select_filter(HINSTANCE hFilter)
{
	char * api_function, * filter_name;
	int n, i;

	/* Is the relevant API function implemented? */
	api_function = "GetNumberOfFilters";
	pGetNumberOfFilters = (int  (WINAPI *)(void))GetProcAddress(hFilter,api_function);
	if (!pGetNumberOfFilters)
	if (!pProcessChannels)
	{
		fprintf(stderr, "\nAPI function %s was not found - assuming a single filter\n", api_function);
		return 0;	/* Default index */
	};

	/* Get number of filted */
	n = pGetNumberOfFilters();
	if (n<1)
	{
		fprintf(stderr, "\nError:	Illigal number (%d) of filters found\n", n);
		return -1;	/* Default index */
	};

	/* Get all filter names and print as an indexed list */
	api_function = "GetFilterNameByIndex";
	pGetFilterNameByIndex = (const char *    (WINAPI *)(const int i))GetProcAddress(hFilter,api_function);
	fprintf(stdout,"Select one of the following filters:\n");
	for (i=0; i<n; i++)
	{
		/* Get name of filter i */
		if (pGetFilterNameByIndex)
			filter_name = (char *)pGetFilterNameByIndex(i);
		else
			filter_name = "";

		/* Print filter index and name */
		fprintf(stdout,"[%d]\t%s\n", i, filter_name);
	};
	fprintf(stdout," >");
	/* Get user selection - if in legal range then return the value */
	fscanf(stdin,"%d", &i);
	if (i>=0 && i<n)
	{
		fprintf(stdout,"Selected filter: [%d]\n", i);
		return i;
	}
	else
	{
		fprintf(stderr,"Error:	Selected filter: [%d] is illegal\n", i);
		return -1;
	}
}

/*
	Before calling the filter - 
	You have to select one of the filters as your filter
	Selection is by zero-based index
	Returns 0 on success
*/
int set_filter(HINSTANCE hFilter, const int i_selected)
{
	char * api_function;

	api_function = "SelectFilterByIndex";
	pSelectFilterByIndex = (const int  (WINAPI *)(const int iFilter))GetProcAddress(hFilter,api_function);
	if (!pSelectFilterByIndex)
	{
		fprintf(stderr, "\nAPI function %s was not found - assuming a single filter\n", api_function);
		return -1;
	};

	/* Select the filter */
	return pSelectFilterByIndex(i_selected);
}

/*
	Gets a vector of 6 channel values and print a corresponding output vector
	In reality, you will have to write a similar function that is called for every joystick vector
	Note that this function must be called only after the correct filter has been selected (SelectFilterByIndex())
	Channel value range is 0-1023.
*/
void run_filter(HINSTANCE hFilter)
{
	char * api_function;
	int in[6],  max, min;
	JS_CHANNELS  * js_filter_in, * js_filter_out;

	/* Init receptor for input data */
	js_filter_in = (JS_CHANNELS  *)malloc(sizeof(JS_CHANNELS));
	max = 1023;	/* Maximum channel value */
	min = 0;	/* Minimum channel value */

	/* Get a 6-channel vector & put it in the input structure */
	fprintf(stdout,"\nEnter 6 channel values separated by spaces \nChannel value range 0-1023\n");
	fscanf(stdin, "%d %d %d %d %d %d", &in[0], &in[1], &in[2], &in[3], &in[4], &in[5]);
	js_filter_in->ch=6;	/* Number of channels */
	js_filter_in->value=in; /* Vector of 6 values */

	/* 
		Run filter: 
		Input channels are in js_filter_in. 
		Output goes to js_filter_out
		min/max are channel value range. SPP values are 0-1023
	*/
	api_function = "ProcessChannels";
	pProcessChannels = (PJS_CHANNELS  (WINAPI *)(PJS_CHANNELS, int max, int min))GetProcAddress(hFilter, api_function);
	js_filter_out = pProcessChannels(js_filter_in, max, min);

	/* Print data */
	if (!js_filter_out)
	{
		fprintf(stderr, "\nAPI function %s failed\n", api_function);
		return;
	}
	else
	{
		fprintf(stdout,"\nInput:\t%d Channels - Ch1=%d  Ch2=%d  Ch3=%d  Ch4=%d  Ch5=%d  Ch6=%d",\
			js_filter_in->ch, in[0], in[1], in[2], in[3], in[4], in[5]);
		fprintf(stdout,"\nOutput:\t%d Channels - Ch1=%d  Ch2=%d  Ch3=%d  Ch4=%d\n",\
			js_filter_out->ch, js_filter_out->value[0], js_filter_out->value[1], js_filter_out->value[2], js_filter_out->value[3]);
	};
}

/*
	Get the version of the DLL
*/
void print_ver(HINSTANCE hFilter)
{
	char * api_function;
	int ver, ver_low, ver_mid, ver_high;

	/* GetDllVersion() - Get the version of the file */
	api_function = "GetDllVersion";
	pGetDllVersion = GetProcAddress(hFilter,api_function);
	if (!pGetDllVersion)
	{
		fprintf(stderr, "\nError:	API function %s was not found\n", api_function);
		return;
	};

	ver = pGetDllVersion();			/* Format: 0x00abcdef*/
	ver_low  = (ver&0x000000FF)>>0; /* ef */
	ver_mid  = (ver&0x0000FF00)>>8; /* cd */
	ver_high = (ver&0x00FF0000)>>16;/* ab */
	fprintf(stdout,"\nFilter File Version: 0.%d.%d.%d\n\n",ver_high, ver_mid, ver_low);
}
