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
