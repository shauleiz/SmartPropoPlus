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
using System.Runtime.InteropServices;

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

        public delegate void vJoyDeviceChanging(uint id);
        public event vJoyDeviceChanging OnvJoyDeviceChanged;

#region General
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

                SelectedvjDevice = new vJoyDevice { vj_DeviceNumber = 0, vj_nAxes = 0, vj_nButtons = 0},
                _vJoyDeviceCollection = new ObservableCollection<vJoyDevice>(),
                CurrentAxisVal = new LevelMonitors(8),
                CurrentJoyInputVal = new LevelMonitors(16),
                CurrentButtonsVal = new vJoyButtonsVal(32), // Number of buttons
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

        #endregion General

#region Audio

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

        //private void P1_Click(object sender, RoutedEventArgs e)
        //{

        //}


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


        //private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        //{

        //}


       #endregion Audio

#region "vJoy Interface"

        // Clean list of vJoy devices
        public void vJoyRemoveAll()
        {
            if (_event._vJoyDeviceCollection.Count > 0)
                _event._vJoyDeviceCollection.Clear();
            _event.SelectedvjDevice = null;
        }

        // Add vJoy Device to list
        public void vJoyDevAdd(uint id)
        {
            // Is item in range
            if ((id <= 0) || (id > 16))
                return;

            // Add item
            vJoyDevice item = new vJoyDevice { vj_DeviceNumber = id};
            _event._vJoyDeviceCollection.Add(item);
        }

        // Set the selected vJoy device
        public void vJoyDevSelect(uint id)
        {
            // Is item in range
            if ((id <= 0) || (id > 16))
                return;

            int count = _event._vJoyDeviceCollection.Count;
            for (int i = 0; i < count; i++)
                if (id == _event._vJoyDeviceCollection[i].vj_DeviceNumber)
                    _event.SelectedvjDevice = _event._vJoyDeviceCollection[i];
        }

        // Update the position of the  progress bar that corresponds to the vJoy axis
        public void SetJoystickAxisData(uint iDev, uint Axis, UInt32 AxisValue)
        {
            // Use data for selected device only
            if (_event.SelectedvjDevice.vj_DeviceNumber != iDev)
                return;
            _event.CurrentAxisVal[Axis - 0x30] = AxisValue; // TODO: HID_USAGE_X
        }


        // Update the position of the progress bar that corresponds to the processed channel
        public void SetProcessedChData(uint iCh, uint data)
        {
            if (_event.CurrentJoyInputVal == null || _event.CurrentJoyInputVal.NumberOfMonitors() <= iCh)
                return;
            _event.CurrentJoyInputVal[iCh] = data;                
        }


        // Set/Reset vJoy device buttons
        public void SetButtonValues(uint id, IntPtr BtnVals)
        {
            // Use data for selected device only
            if (_event != null && _event.SelectedvjDevice.vj_DeviceNumber != id)
                return;

            // Get the number of buttons that actually active
            uint nButtons = 0;
            if (_event != null && _event.CurrentvjCtrl != null && _event.CurrentvjCtrl.nButtons <= 128)
                nButtons = _event.CurrentvjCtrl.nButtons;

            // Copy unmanaged array to managed array
            byte[] BtnValsArray = new byte[nButtons];
            Marshal.Copy(BtnVals, BtnValsArray, 0, BtnValsArray.Length);

            // Copy managed array to model object
            uint count=0;
            foreach (byte element in BtnValsArray)
            {           
                _event.CurrentButtonsVal[count] = (element==0) ? false : true;
                count += 1;
            }

        }

        // Get the new selected vJoy device as the user selected from the combo box
        //  - Call event OnvJoyDeviceChanged
        private void vJoyDeviceCB_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (_event.SelectedvjDevice != null)
            {
                uint selected_id = _event.SelectedvjDevice.vj_DeviceNumber;
                OnvJoyDeviceChanged(selected_id);
            };
        }


        // Enable/disable controls according to vJoy device settings
        public void EnableControls(uint id, Mcontrols ctrl)
        {
            int len = ctrl.axis.Length;
            if (_event.CurrentvjCtrl != null)
                _event.CurrentvjCtrl.axis = new bool[len];

            ////// Verify correct vJoy device
            if (_event.SelectedvjDevice != null && _event.SelectedvjDevice.vj_DeviceNumber != id)
                return;

            _event.CurrentvjCtrl = ctrl;
            _event.CurrentvjCtrl.nButtons = ctrl.nButtons;
            _event.CurrentvjCtrl.axis = ctrl.axis;
            
        }

#endregion // "vJoy Interface"


    }

#region Data Structures
    public class AudioLine
    {
        public string DeviceName { get; set; }
        public string EmptyStr { get; set; }
        public Brush Fill { get; set; }
        public string ID { get; set; }
        public uint LevelLeft { get; set; }
        public uint LevelRight { get; set; }
    }

    public class vJoyDevice
    {
        public uint vj_DeviceNumber { get; set; }
        public uint vj_nButtons { get; set; }
        public uint vj_nAxes { get; set; }
        //public bool vj_Selected { get; set; }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public class Mcontrols
    {
        public uint nButtons { get; set; }	// Number of buttons
        public bool[] axis { get; set; }    // Axis Exists
        public Mcontrols() {axis  = new bool[8];}
    }


    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public class vJoyDaviceState
    {
        public int Id;
        public UInt32[] AxisVal  { get; set; }
        public bool[] ButtonVal  { get; set; }

        public vJoyDaviceState()
        {
            Id = -1;
            AxisVal = new UInt32[8];
            ButtonVal = new bool[32];
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public class vJoyAxisVal : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

        public UInt32[] AxisVal { get; set; }
        public vJoyAxisVal() { AxisVal = new UInt32[8] { 0, 0, 0, 0, 0, 0, 0, 0 }; }

        public UInt32 this[uint index]
        {
            get { return AxisVal[index]; }
            set
            {
                AxisVal[index] = value;
                OnPropertyChanged(Binding.IndexerName);
            }
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public class LevelMonitors : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

        public UInt32[] LevelVal { get; set; }
        public LevelMonitors() { LevelVal = new UInt32[8] { 0, 0, 0, 0, 0, 0, 0, 0 }; }
        public LevelMonitors(int size) { LevelVal = new UInt32[size]; }
        public int NumberOfMonitors() { return LevelVal.Length; }

        public UInt32 this[uint index]
        {
            get { return LevelVal[index]; }
            set
            {
                LevelVal[index] = value;
                OnPropertyChanged(Binding.IndexerName);
            }
        }
    }


    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public class vJoyButtonsVal : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

        public bool[] ButtonsVal { get; set; }
        public vJoyButtonsVal() { ButtonsVal = new bool[32]; }
        public vJoyButtonsVal(uint size) { ButtonsVal = new bool[size]; }

        public bool this[uint index]
        {
            get { return ButtonsVal[index]; }
            set
            {
                ButtonsVal[index] = value;
                OnPropertyChanged(Binding.IndexerName);
            }
        }
    }


#endregion Data Structures

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


    /// <summary>
    /// This converter does nothing except breaking the
    /// debugger into the convert method
    /// From: http://wpftutorial.net/DebugDataBinding.html
    /// </summary>
    public class DatabindingDebugConverter : IValueConverter
    {
        public object Convert(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            Debugger.Break();
            return value;
        }

        public object ConvertBack(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            Debugger.Break();
            return value;
        }
    }
    // Unsigned Integer to to boolean - The Unsigned Integer represents a vJoy axis (0=X ... 7=SL1)
    // The bollean value is the boolean value in the array of enabled axis in CurrentvjCtrl.Axis
#if false
    [ValueConversion(typeof(uint), typeof(bool))]
    public class UintToEnabledAxis : IValueConverter
    {
        public uint Axis { get; set; }
        public UintToEnabledAxis() { Axis = 0; }

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (Axis > 7)
                return false;

            return _event.CurrentvjCtrl.Axis[Axis];
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return DependencyProperty.UnsetValue;
        }

    }

    
#endif
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