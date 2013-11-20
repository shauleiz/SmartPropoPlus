﻿using System;
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
        public EventModel _event;
        public delegate void WinMoving(double Left, double Top);
        public event WinMoving OnMove;
        public delegate void AudioChanging(int bitrate, char channel);
        public event AudioChanging OnAudioChanged;
        public delegate void AudioAutoChanging(bool AutoBitrateChecked, bool AutoChannelChecked);
        public event AudioAutoChanging OnAudioAutoChanged;

        public CtrlWindow()
        {
            BindingErrorTraceListener.SetTrace();
            InitializeComponent();

            // create a model object
            _event = new EventModel()
            {
                SelectedJack = new AudioLine { DeviceName = "-- Not Implemented -- ", LevelLeft = 0, LevelRight = 0, EmptyStr = "-", Fill = new SolidColorBrush() },
                _AudioDeviceCollection = new ObservableCollection<AudioLine>(),
                AudioBitrate = 0,
                AudioChannel = 'U',
                IsNotAutoBitrate = true,
                IsNotAutoChannel = true,
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

#if false
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            _event.SelectedJack = _event.SelectedJack.DeviceName.ToLower();
            // Remove fifth item
            int iRem = 4;
            if (_event._AudioDeviceCollection.Count > iRem)
                _event._AudioDeviceCollection.RemoveAt(iRem);

            // Select Second item
            Audio_LB.SelectedIndex = 1;

            // Scroll to selected item
            Audio_LB.ScrollIntoView(Audio_LB.SelectedItem);
            ListViewItem item = Audio_LB.ItemContainerGenerator.ContainerFromItem(Audio_LB.SelectedIndex) as ListViewItem;
            if (item != null)
                item.Focus();

            Insert_Jack("{MyId}", "DDDevName", 0, 0, true, 0);
        }
        
#endif

        // Clean list of Audio devices
        public void CleanAudioList()
        {
            if (_event._AudioDeviceCollection.Count > 0)
                _event._AudioDeviceCollection.Clear();
            _event.SelectedJack = null;
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
        public void Insert_Jack(string JackID, string DevName, uint LLevel, uint RLevel, bool Selected, uint color)
        {
            Color csColor = Color.FromRgb((byte)((color & 0xff0000) >> 0x10), (byte)((color & 0xff00) >> 8), (byte)(color & 0xff));
            AudioLine item = new AudioLine { ID = JackID, DeviceName = DevName, LevelLeft = LLevel, LevelRight = RLevel, EmptyStr = "-", Fill = new SolidColorBrush(csColor) };
            _event._AudioDeviceCollection.Add(item);

            if (Selected)
            {
                _event.SelectedJack = item;
                int index = _event._AudioDeviceCollection.IndexOf(item);
                Audio_LB.SelectedIndex = index;
                Audio_LB.ScrollIntoView(Audio_LB.SelectedItem);
                ListViewItem lvitem = Audio_LB.ItemContainerGenerator.ContainerFromItem(index) as ListViewItem;
                if (lvitem != null)
                    lvitem.Focus();
            };
        }

        // Display the audio levels of channels (Left/Right)
        // Levels are in the range 0-100
        public void SetAudioLevels_Jack(string JackID, uint Left, uint Right)
        {
            // Find jack by ID
            int index = -1;
            int count = _event._AudioDeviceCollection.Count;
            for (int i = 0; i < count; i++)
                if (_event._AudioDeviceCollection[i].ID.Equals(JackID))
                    index = i;

            // Not found
            if (index<0)
                return;

            // Update levels
            _event._AudioDeviceCollection[index].LevelLeft = Left;
            _event._AudioDeviceCollection[index].LevelRight = Right;

            // Notify of the change
            _event.SelectedAudioDevice = null;
            _event.SelectedAudioDevice = _event._AudioDeviceCollection[index];

            // Refresh view
            ICollectionView view = CollectionViewSource.GetDefaultView(_event._AudioDeviceCollection);
            view.Refresh();
        }

        private void P1_Click(object sender, RoutedEventArgs e)
        {

        }


        // One of the Audio radio buttons was checked - Call event OnAudioChanged
        private void Audio_RB_Checked(object sender, RoutedEventArgs e)
        {
            if (OnAudioChanged != null)
                OnAudioChanged(_event.AudioBitrate, _event.AudioChannel);
        }

        private void Audio_CheckBox_Click(object sender, RoutedEventArgs e)
        {
                OnAudioAutoChanged(!_event.IsNotAutoBitrate, !_event.IsNotAutoChannel);

        }


    }

    public class AudioLine
    {
        public string DeviceName { get; set; }
 //       public string LR_LevelsStr { get; set; }
        public string EmptyStr { get; set; }
        public Brush Fill { get; set; }
        public string ID { get; set; }
        public uint LevelLeft { get; set; }
        public uint LevelRight { get; set; }
    }


    #region Converters
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

    // Int to string converter - adds 'Upendix' to bitrate value (default: "Bit")
    [ValueConversion(typeof(int), typeof(string))]
    public sealed class IntToBitrateStrConverter : IValueConverter
    {
        public string Upendix { get; set; }

        public IntToBitrateStrConverter() { Upendix = "Bit"; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value.ToString() + Upendix;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            string str = value as string;
            return System.Convert.ToInt32(str);
        }
    }

    // Int to bool converter - Returns true if value is equal to 'Target' (default: 8)
    [ValueConversion(typeof(int), typeof(bool))]
    public sealed class IntToBitrateBoolConverter : IValueConverter
    {
        public int Target { get; set; }

        public IntToBitrateBoolConverter() { Target = 8; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return Equals(value, Target);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (Equals(value, true))
                return Target;
            else
                return DependencyProperty.UnsetValue;
        }
    }

    // Char to string converter - Replace L/R/M with more elaborate strings
    [ValueConversion(typeof(char), typeof(string))]
    public sealed class CharToChannelStrConverter : IValueConverter
    {
        public string Left { get; set; }
        public string Right { get; set; }
        public string Mono { get; set; }
        public string Unknn { get; set; }

        public CharToChannelStrConverter() { Left = "Left Channel"; Right = "Right Channel"; Mono = "Mono"; Unknn = "Channel Unknown"; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            string ch = value.ToString();
            switch (ch[0])
            {
                case 'L':
                    return Left;
                case 'R':
                    return Right;
                case 'M':
                    return Mono;
                default:
                    return Unknn;
            };
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (Equals(value, Left))
                return 'L';
            if (Equals(value, Right))
                return 'R';
            if (Equals(value, Mono))
                return 'M';

            return null;
        }
    }

    // Char to bool converter - Returns true if value is equal to 'Target' (default: 'L')
    [ValueConversion(typeof(int), typeof(bool))]
    public sealed class CharToChannelBoolConverter : IValueConverter
    {
        public char Target { get; set; }

        public CharToChannelBoolConverter() { Target = 'L'; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return Equals(value, Target);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (Equals(value, true))
                return Target;
            else
                return DependencyProperty.UnsetValue;
        }
    }

    // Boolean NOT converter - Based on http://stackoverflow.com/questions/1039636/how-to-bind-inverse-boolean-properties-in-wpf
    [ValueConversion(typeof(bool), typeof(bool))]
    public class InverseBooleanConverter : IValueConverter
    {

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return !(bool)value;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            return !(bool)value;
        }
    }
    
    #endregion // Converters

} // namespace CtrlWindowNS


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