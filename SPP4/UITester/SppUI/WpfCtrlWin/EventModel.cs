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

namespace CtrlWindowNS
{
    /// <summary>
    /// A simple model object that represents an event
    /// </summary>
    public class EventModel : INotifyPropertyChanged
    {
        private string _selected_jack;

        /// <summary>
        /// Gets / sets the event Selected audio jack
        /// </summary>
        public string SelectedJack
        {
            get { return _selected_jack; }
            set
            {
                if (value == _selected_jack)
                    return;

                _selected_jack = value;
                Dispatcher.CurrentDispatcher.BeginInvoke(
    new Action<String>(OnPropertyChanged),
    DispatcherPriority.DataBind, "SelectedJack");

            }
        }

#if false
        private DateTime _date;

        /// <summary>
        /// Gets / sets the date of this event
        /// </summary>
        public DateTime Date
        {
            get { return _date; }
            set
            {
                if (value == _date)
                    return;

                _date = value;
                OnPropertyChanged("Date");
            }
        }
        
#endif
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
