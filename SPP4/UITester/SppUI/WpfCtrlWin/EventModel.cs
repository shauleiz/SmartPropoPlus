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

        // Selected Audio Jack - Property is "SelectedJack"
        private string _selected_jack;
        public string SelectedJack
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

        // Auto channel chaeck box - NOT checked
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

        // Audio list box headers
        public string DaviceStr = "Device";

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
