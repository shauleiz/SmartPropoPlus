using System;
using System.Collections.Generic;
//using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Data;
using System.Globalization;
using System.Diagnostics;
using System.Text;
using System.ComponentModel;
using System.Collections.ObjectModel;

namespace CtrlWindowNS
{
    public partial class CtrlWindow : Window
    {
        private EventModel _event;
        public delegate void ClockMoving(double Left, double Top);
        public event ClockMoving OnMove;

        public CtrlWindow()
        {
            BindingErrorTraceListener.SetTrace();
            InitializeComponent();    

            // create a model object
            _event = new EventModel()
            {
                SelectedJack = "-- No audio jack Selected --",
                _AudioDeviceCollection = new ObservableCollection<AudioLine>(),
                AudioBitrate = "0 Bit",
                AudioChannel = "Unknown channel",
            };


            this.DataContext = _event;

        }

        void NonRectangularWindow_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.DragMove();
        }

        private void LocChanged(object sender, System.EventArgs e)
        {
            if (OnMove != null)
                OnMove(Left, Top);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            _event.SelectedJack = _event.SelectedJack.ToLower();
            // Remove fifth item
            int iRem = 4;
             if (_event._AudioDeviceCollection.Count> iRem)
                 _event._AudioDeviceCollection.RemoveAt(iRem);

            // Select Second item
            Audio_LB.SelectedIndex = 1;

            // Scroll to selected item
            Audio_LB.ScrollIntoView(Audio_LB.SelectedItem);
            ListViewItem item = Audio_LB.ItemContainerGenerator.ContainerFromItem(Audio_LB.SelectedIndex) as ListViewItem;
            if (item != null)
                item.Focus();

            Insert_Jack("DDDevName", "DDDlevels", true);
        }

        public void Set_TB_SelectedJack(string intext)
        {
            _event.SelectedJack = intext;
        }

        // Clean list of Audio devices
        public void CleanAudioList()
        {
            if (_event._AudioDeviceCollection.Count>0)
                _event._AudioDeviceCollection.Clear();
        }

        /// <summary>
        ///  Insert an audio device (= jack) to _AudioDeviceCollection
        ///  If marked as Selected then:
        ///  1. Put the device name in the control window
        ///  2. Mark it as selected in the listview and scroll to it
        /// </summary>
        /// <param name="DevName"> Display name of the device</param>
        /// <param name="levels"> Audio levels as string</param>
        /// <param name="Selected">If true: This is the selected device</param>
        public void Insert_Jack(string DevName, string levels, bool Selected)
        {
            AudioLine item = new AudioLine { DeviceName = DevName, LR_LevelsStr = levels, EmptyStr = "-" };
            _event._AudioDeviceCollection.Add(item);

            if (Selected)
            {
                Set_TB_SelectedJack(DevName);
                int index = _event._AudioDeviceCollection.IndexOf(item);
                Audio_LB.SelectedIndex = index;
                Audio_LB.ScrollIntoView(Audio_LB.SelectedItem);
                ListViewItem lvitem = Audio_LB.ItemContainerGenerator.ContainerFromItem(index) as ListViewItem;
                if (lvitem != null) 
                    lvitem.Focus();
           };
        }

        /// <summary>
        /// Updates channel info - selected channel
        /// </summary>
        /// <param name="Channel"> Chan be 'L'/'R'/'M'</param>
        public void Set_Channel(char Channel)
        {
            switch (Channel)
            {
                case 'L':
                    _event.AudioChannel = "Left Channel";
                    break;
                case 'R':
                    _event.AudioChannel = "Right Channel";
                    break;
                case 'M':
                    _event.AudioChannel = "Mono";
                    break;
                default:
                    _event.AudioChannel = "Unknown Channel";
                    break;
            };
        }

        public void Set_Bitrate(int br)
        {
            _event.AudioBitrate = br.ToString()+"Bit";
        }

        private void P1_Click(object sender, RoutedEventArgs e)
        {

        }

        private void Audio_Click(object sender, RoutedEventArgs e)
        {

        }

        private void Audio_LB_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

        }

       
    }

    public class AudioLine
    {
        public string DeviceName { get; set; }
        public string LR_LevelsStr { get; set; }
        public string EmptyStr  { get; set; }
    }


    // Boolean to Color converter - may be overriden in the dictionary
  [ValueConversion(typeof(bool), typeof(Brush))]
    public sealed class BoolToBorderBrushColorConverter : IValueConverter
    {
        public Brush TrueValue { get; set; }
        public Brush FalseValue { get; set; }

        public BoolToBorderBrushColorConverter()
        {
            // set defaults
            TrueValue = Brushes.GreenYellow;
            FalseValue = Brushes.Red;
        }

        public object Convert(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            if (!(value is bool))
                return null;
            return (bool)value ? TrueValue : FalseValue;
        }

        public object ConvertBack(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            if (Equals(value, TrueValue))
                return true;
            if (Equals(value, FalseValue))
                return false;
            return null;
        }
    }
   
}

// Detecting Binding Errors
// Based on: http://tech.pro/tutorial/940/wpf-snippet-detecting-binding-errors
namespace CtrlWindowNS
{
  public class BindingErrorTraceListener : DefaultTraceListener
  {
    private static BindingErrorTraceListener _Listener;

    public static void SetTrace()
    { SetTrace(SourceLevels.Error, TraceOptions.None); }

    public static void SetTrace(SourceLevels level, TraceOptions options)
    {
      if (_Listener == null)
      {
        _Listener = new BindingErrorTraceListener();
        PresentationTraceSources.DataBindingSource.Listeners.Add(_Listener);
      }

      _Listener.TraceOutputOptions = options;
      PresentationTraceSources.DataBindingSource.Switch.Level = level;
    }

    public static void CloseTrace()
    {
      if (_Listener == null)
      { return; }

      _Listener.Flush();
      _Listener.Close();
      PresentationTraceSources.DataBindingSource.Listeners.Remove(_Listener);
      _Listener = null;
    }



    private StringBuilder _Message = new StringBuilder();

    private BindingErrorTraceListener()
    { }

    public override void Write(string message)
    { _Message.Append(message); }

    public override void WriteLine(string message)
    {
      _Message.Append(message);

      var final = _Message.ToString();
      _Message.Length = 0;

      MessageBox.Show(final, "Binding Error", MessageBoxButton.OK, 
        MessageBoxImage.Error);
    }
  }
}