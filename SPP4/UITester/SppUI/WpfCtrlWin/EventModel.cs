using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.ComponentModel;
using System.Windows.Threading;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows.Data;

namespace CtrlWindowNS
{
    /// <summary>
    /// A simple model object that represents an event
    /// </summary>
    public class EventModel : INotifyPropertyChanged
    {

        /// <summary>
        /// Gets / sets the event Selected audio jack
        /// </summary>
        // From http://stackoverflow.com/questions/1315621/implementing-inotifypropertychanged-does-a-better-way-exist
        protected bool SetField<T>(ref T field, T value, string propertyName)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

#region Audio

        // Selected Audio Jack
        private AudioLine _selected_jack;
        public AudioLine SelectedJack
        {
            get { return _selected_jack; }
            set { SetField(ref _selected_jack, value, "SelectedJack"); }
        }

        // Set Left/Right/Mono audio channel
        private char _audio_channel;
        public char AudioChannel
        {
            get { return _audio_channel; }
            set { SetField(ref _audio_channel, value, "AudioChannel"); }
        }

        // Set audio channel Bit Rate
        private int _audio_bitrate;
        public int AudioBitrate
        {
            get { return _audio_bitrate; }
            set { SetField(ref _audio_bitrate, value, "AudioBitrate"); }
        }

        // Auto bitrate chaeck box - NOT checked
        private bool _is_Not_AutoBitrate;
        public bool IsNotAutoBitrate
        {
            get { return _is_Not_AutoBitrate; }
            set { SetField(ref _is_Not_AutoBitrate, value, "IsNotAutoBitrate"); }
        }

        // Auto channel chack box - NOT checked
        private bool _is_Not_AutoChannel;
        public bool IsNotAutoChannel
        {
            get { return _is_Not_AutoChannel; }
            set { SetField(ref _is_Not_AutoChannel, value, "IsNotAutoChannel"); }
        }


        // Audio: List of audio lines
        public ObservableCollection<AudioLine> _AudioDeviceCollection;
        public ObservableCollection<AudioLine> AudioDeviceCollection
        { 
            get { return _AudioDeviceCollection; } 
        }

        // Audio list - Selected item
        private AudioLine _SelectedAudioDevice;
        public  AudioLine SelectedAudioDevice
        {
            get
            { return _SelectedAudioDevice;}

            set 
            {
                SetField(ref _SelectedAudioDevice, value, "SelectedAudioDevice");
            }
        }

        // Audio list box headers
        public string DaviceStr = "Device";

#endregion Audio

#region "vJoy Interface"
        // vJoy: List of available vJoy devices
        public ObservableCollection<vJoyDevice> _vJoyDeviceCollection;
        public ObservableCollection<vJoyDevice> vJoyDeviceCollection
        {
            get { return _vJoyDeviceCollection; }
            set { SetField(ref _vJoyDeviceCollection, value, "vJoyDeviceCollection"); }
        }

        // Selected vJoy device
        private vJoyDevice _selected_vjDevice;
        public vJoyDevice SelectedvjDevice
        {
            get { return _selected_vjDevice; }
            set { SetField(ref _selected_vjDevice, value, "SelectedvjDevice"); }
        }

        // Input channel that the mouse hover above
        private string _hovered_vjInput;
        public string HoveredvjInput
        {
            get { return _hovered_vjInput; }
            set { SetField(ref _hovered_vjInput, value, "HoveredvjInput"); }
        }

        // Set number of existing buttons in the currently selected vJoy Device
        private Mcontrols _current_vjctrl;
        public  Mcontrols CurrentvjCtrl
        {
            get { return _current_vjctrl; }
            set 
            {
                SetField(ref _current_vjctrl, value, "CurrentvjCtrl");

                _current_vjctrl.nButtons = value.nButtons;
                OnPropertyChanged("CurrentvjCtrl.nButtons");

                int len = _current_vjctrl.axis.Length;
                for (int i = 0; i < len; i++ )
                {
                    _current_vjctrl.axis[i] = value.axis[i];
                }
                OnPropertyChanged(Binding.IndexerName);
            }
        }


        public LevelMonitors CurrentAxisVal { get; set; }
        public LevelMonitors CurrentJoyInputVal { get; set; }

        public ObservableCollection<LevelMonitor> _vJoyAxisCollection;
        public ObservableCollection<LevelMonitor> vJoyAxisCollection
        {
            get { return _vJoyAxisCollection; }
            set { SetField(ref _vJoyAxisCollection, value, "vJoyAxisCollection"); }
        }

        public ObservableCollection<LevelMonitor> _vJoyInputCollection;
        public ObservableCollection<LevelMonitor> vJoyInputCollection
        {
            get { return _vJoyInputCollection; }
            set { SetField(ref _vJoyInputCollection, value, "vJoyInputChannel"); }
        }

        // BoolMonitor
        public ObservableCollection<BoolMonitor> _vJoyButtonCollection;
        public ObservableCollection<BoolMonitor> vJoyButtonCollection
        {
            get { return _vJoyButtonCollection; }
            set { SetField(ref _vJoyButtonCollection, value, "vJoyButtonCollection"); }
        }

        //public vJoyButtonsVal CurrentButtonsVal { get; set; }

#endregion "vJoy Interface"

#region Decoder

        //Decoder: List of modulations
        public ObservableCollection<DecoderItem> _DecoderCollection;
        public ObservableCollection<DecoderItem> DecoderCollection
        {
            get { return _DecoderCollection; }
        }

        // Decoder list - Selected item
        private DecoderItem _SelectedDecoder;
        public DecoderItem SelectedDecoder
        {
            get
            { return _SelectedDecoder; }

            set
            {
                SetField(ref _SelectedDecoder, value, "SelectedDecoder");
            }
        }

#endregion Decoder
    }
}
