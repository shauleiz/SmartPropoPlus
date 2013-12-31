using System;
using System.Collections.Generic;
//using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
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
        public delegate void vJoyMapAxisChanging(int AxisId, int NewSrcCh);
        public event vJoyMapAxisChanging OnvJoyMapAxisChanged;

        public delegate void DecoderChanging(string Type);
        public event DecoderChanging OnDecoderChanged;

#region General
        public CtrlWindow()
        {
            BindingErrorTraceListener.SetTrace();
            InitializeComponent();

            CtrlWindowInit();

            // Data initialization
            InitvJoyAxes();
            InitvJoyInputChs(16);
            InitvJoyButtons(32);

            this.DataContext = _event;
        }

        private void CtrlWindowInit()
        {
            // create a model object
            _event = new EventModel()
            {
                SelectedJack = new AudioLine { DeviceName = "-- Not Implemented -- ", LevelLeft = 0, LevelRight = 0, EmptyStr = "-", Fill = new SolidColorBrush() },
                _AudioDeviceCollection = new ObservableCollection<AudioLine>(),
                AudioBitrate = 0,
                AudioChannel = 'U',
                IsNotAutoBitrate = true,
                IsNotAutoChannel = true,
                HoveredvjInput = "0",

                SelectedvjDevice = new vJoyDevice { vj_DeviceNumber = 0, vj_nAxes = 0, vj_nButtons = 0 },
                _vJoyDeviceCollection = new ObservableCollection<vJoyDevice>(),

                _vJoyAxisCollection = new ObservableCollection<LevelMonitor>(),
                _vJoyInputCollection = new ObservableCollection<LevelMonitor>(),
                _vJoyButtonCollection = new ObservableCollection<BoolMonitor>(),

                _DecoderCollection = new ObservableCollection<DecoderItem>(),

                SelectedFilter = new FilterItem {Name = "", Id = -1},
                FilterFileName = "" ,
                _FilterCollection = new ObservableCollection<FilterItem>(),

            };
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
                //Audio_LB.SelectedIndex = index;
                //Audio_LB.ScrollIntoView(Audio_LB.SelectedItem);
                //ListViewItem lvitem = Audio_LB.ItemContainerGenerator.ContainerFromItem(index) as ListViewItem;
                //if (lvitem != null)
                //    lvitem.Focus();
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

            //// TODO: Remove
            //if (_event.CurrentvjCtrl != null && _event.CurrentvjCtrl.axis[Axis - 0x30])
            //    _event.CurrentAxisVal[Axis - 0x30] = AxisValue; // TODO: HID_USAGE_X

            // New
            if (_event._vJoyAxisCollection != null && _event._vJoyAxisCollection[(int)Axis - 0x30] != null)
                _event._vJoyAxisCollection[(int)Axis - 0x30].Level =  AxisValue; // TODO: HID_USAGE_X

            // Refresh view
            ICollectionView view = CollectionViewSource.GetDefaultView(_event._vJoyAxisCollection);
            view.Refresh();
        }


        // Update the position of the progress bar that corresponds to the processed channel
        public void SetProcessedChData(uint iCh, uint data)
        {
            if (_event._vJoyInputCollection == null || _event._vJoyInputCollection.Count <= iCh)
                return;
            _event._vJoyInputCollection[(int)iCh].Level = data;

            // Refresh view
            ICollectionView view = CollectionViewSource.GetDefaultView(_event._vJoyInputCollection);
            view.Refresh();
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
                _event._vJoyButtonCollection[(int)count].Level  = (element==0) ? false : true;
                count += 1;
            }
            // Refresh view
            ICollectionView view = CollectionViewSource.GetDefaultView(_event._vJoyButtonCollection);
            view.Refresh();

        }

        // Fill-in mapping data
        public void SetMappingData(ulong AxisMap, IntPtr ButtonMap)
        {
            /// Axes - Going over AxisMap.
            /// Every nibble is an entry represents target axis (by location) and holds value of input channel.
            /// Every entry is loaded into the mapping source in the target axis item
            /// and added to the array of target axes in the source channel
            /// 

            int nAxes = _event._vJoyAxisCollection.Count;
            int src = 0;
            for (int i = 0; i < nAxes; i++)
            {
                src = (int)((AxisMap >> (i * 4)) & 0xF);
                if (src < 1 || src > _event._vJoyInputCollection.Count)
                    continue;
                _event._vJoyAxisCollection[nAxes-i-1].MapSource = src;
            };

            // Refresh values of targets in the Input Channels collection
            RefreshTargetMapping();

            // Refresh view
            ICollectionView view1 = CollectionViewSource.GetDefaultView(_event._vJoyAxisCollection);
            view1.Refresh();
            
        }

        // Set number of raw (transmitter) channels
        public void SetNumberRawCh(uint nCh)
        {
            _event.nTransmitterChannels = nCh;
        }

        /// <summary>
        /// Refresh all Map Target arrays in Input Collection
        /// </summary>
        private void RefreshTargetMapping()
        {
            // Reset
            foreach (LevelMonitor element in _event._vJoyInputCollection)
                element.UnMapAllTargets();

            // Go over all Axes and buttons and get the sources
            // For every source found, add target to its target list

            // Axes
            foreach (LevelMonitor element in _event._vJoyAxisCollection)
            {
                string sSrc = element.MapSource.ToString();
                int iSrc = 0;
                bool converted = Int32.TryParse(sSrc, out iSrc);
                if (converted && iSrc > 0)
                    _event._vJoyInputCollection[iSrc - 1].MapTarget(element.Id);
            };

            // Buttons
            foreach (LevelMonitor element in _event._vJoyButtonCollection)
            {
                string sSrc = element.MapSource.ToString();
                int iSrc = 0;
                bool converted = Int32.TryParse(sSrc, out iSrc);
                if (converted && iSrc > 0)
                    _event._vJoyInputCollection[iSrc - 1].MapTarget(element.MapSource);
            };

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

            // TODO: Remove later
            _event.CurrentvjCtrl = ctrl;
            _event.CurrentvjCtrl.nButtons = ctrl.nButtons;
            _event.CurrentvjCtrl.axis = ctrl.axis;

            if (_event._vJoyAxisCollection != null)
            {
                int count = ctrl.axis.Length;
                for (int i = 0; i < count; i++)
                {
                    if (i >= _event._vJoyAxisCollection.Count)
                        break;
                    _event._vJoyAxisCollection[i].Implemented = ctrl.axis[i];
                }

                // Refresh view
                ICollectionView view = CollectionViewSource.GetDefaultView(_event._vJoyAxisCollection);
                view.Refresh();
            }

            if (_event._vJoyButtonCollection != null)
            {
                uint i=0;
                foreach (BoolMonitor element in _event._vJoyButtonCollection)
                {
                    if (i < ctrl.nButtons)
                        element.Implemented = true;
                    else
                        element.Implemented = false;
                    i++;
                }

                // Refresh view
                ICollectionView view = CollectionViewSource.GetDefaultView(_event._vJoyButtonCollection);
                view.Refresh();
            }

        }

        // Initialize vJoy Axes data collection
        public void InitvJoyAxes()
        {
            if (_event == null || _event._vJoyAxisCollection == null)
                return;
            LevelMonitor item;
            _event._vJoyAxisCollection.Clear();
            item = new LevelMonitor(0, "X");
            _event._vJoyAxisCollection.Add(item);
            item = new LevelMonitor(1, "Y");
            _event._vJoyAxisCollection.Add(item);
            item = new LevelMonitor(2, "Z");
            _event._vJoyAxisCollection.Add(item);
            item = new LevelMonitor(3, "Rx");
            _event._vJoyAxisCollection.Add(item);
            item = new LevelMonitor(4, "Ry");
            _event._vJoyAxisCollection.Add(item);
            item = new LevelMonitor(5, "Rz");
            _event._vJoyAxisCollection.Add(item);
            item = new LevelMonitor(6, "SL0");
            _event._vJoyAxisCollection.Add(item);
            item = new LevelMonitor(7, "SL1");
            _event._vJoyAxisCollection.Add(item);
        }

        // Initialize data collection of input channels to vJoy
        public void InitvJoyInputChs(int nOfChs)
        {
            if (_event == null || _event._vJoyInputCollection == null)
                return;
            
            LevelMonitor item;

            _event._vJoyInputCollection.Clear();

            for (int i = 0; i < nOfChs; i++)
            {
                item = new LevelMonitor(i, (i + 1).ToString());
                _event._vJoyInputCollection.Add(item);
            }
        }

        // Initialize data collection of vJoy buttons
        public void InitvJoyButtons(int nOfBtns)
        { 
            if (_event == null || _event._vJoyButtonCollection == null)
                return;

            BoolMonitor item;
            for (int i = 0; i < nOfBtns; i++)
            {
                item = new BoolMonitor(i, (i + 1).ToString());
                _event._vJoyButtonCollection.Add(item);
            }
        }

        /// <summary>
        /// Handler to Event TextChanged
        /// Fired when the user changes the text of the mapping source of one of the vJoy axes
        /// If input is valid then call SendAxisMappingData()
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void MapSourceChanged(object sender, TextChangedEventArgs e)
        {

            // Get the new value from the Text Box
            var textBox = sender as TextBox;
            string src = textBox.Text;
            int iSrc=0;
            bool converted = Int32.TryParse(src, out iSrc);

            // Get the Name of the target axis from the Sender
            UIElementCollection Grd = ((sender as FrameworkElement).Parent as Grid).Children;
            string Name = "";
            foreach (object o in Grd)
            {
                if (o is WpfCtrlWin.ChannelCtrl)
                    Name = (o as WpfCtrlWin.ChannelCtrl).ChannelName;
            }

            // Call the actual method that passes the change to the SppTopWin
            if (textBox.Name.Equals("MapSourceCh") && converted && iSrc>0)
                SendAxisMappingData(iSrc, Name);
        }

        private void MapSourceKeyDown(object sender, KeyEventArgs e)
        {
            // Process if ENTER
            if (e.Key == Key.Return)
                MapSourceChanged(sender, null);        
        }

        // Send vJoy axis mapping information to the CLI wrapper
        private void SendAxisMappingData(int NewSrc, string TargetName)
        {

            // Find the Axis Id by Name
            int id = -1;
            foreach (LevelMonitor element in _event._vJoyAxisCollection)
            {
                if (element.Name.Equals(TargetName))
                {
                    id = element.Id;
                    break;
                }
            }
            if (id < 0)
                return;
            // Invoke the event handler to carry the info to the CLI wrapper (SppTopWin)
            OnvJoyMapAxisChanged(id, NewSrc);

        }

#endregion // "vJoy Interface"

#region Decoder Interface
        /// Decoding is also referred to as "modulation"
        /// Each decoder is defined by its Type (e.g. AIR1) its subtype (e.g. PCM) and name (This is the display text)
        /// 


        /// Add modulation entry to the collection of modulations
        /// Testing that entry does not exist
        public void AddModulation(string name, string type, string subtype)
        {
            // Search for type in collection
            foreach (DecoderItem item in _event.DecoderCollection)
            {
                if (item.Name.Equals(name))
                    return;
            }

            // Not found, so create item and add to collection
            DecoderItem decoder = new DecoderItem { Name = name, Type = type, Subtype = subtype };
            _event.DecoderCollection.Add(decoder);

        }

        /// <summary>
        /// Set the selected modulation entry
        /// </summary>
        /// <param name="SelType"></param>
        public void SetSelectedModulation(string SelType)
        {
            // Search for type in collection
            foreach (DecoderItem item in _event.DecoderCollection)
            {
                if (item.Type.Equals(SelType))
                {
                    _event.SelectedDecoder = item;
                    break;
                }
            }
             
        }

#endregion Decoder Interface

#region Filter Interface

        /// <summary>
        /// Sets the current filter file name (Display name)
        /// If string is empty then clear data
        /// </summary>
        /// <param name="FilterFileName">Display Name of Filter file (DLL)</param>
        public void SetFilterFileName(string Name)
        {
            // Valid?
            if (Name.Length > 0)
                _event.FilterFileName = Name;
            else
                _event.FilterFileName = "";
        }

        /// <summary>
        /// Reset the collection of filters - call before populating (when switching filter file)
        /// Sets a dummy "- Select FIlter -" first item
        /// Selects the dummy first item
        /// </summary>
        /// <param name="n">Ignored</param>
        public void ResetFilterCollection(int n)
        {
            _event._FilterCollection.Clear();
            FilterItem item = new FilterItem() { Id = -1, Name = "- Select Filter -" };
            _event._FilterCollection.Add(item);
            SelFilter(-1);
        }

        /// <summary>
        /// Updates property IsEnabledFilter
        /// </summary>
        /// <param name="enable"></param>
        //public void IsEnabledFilter(bool enable)
        //{
        //    _event.IsEnabledFilter = enable;
        //}

        /// <summary>
        /// Add a filter to the collection of filters
        /// Note that a collection represents the filters encapsulated in a specific Filter File
        /// Every filter is identified by a unique ID and a Filter (Friendly) Name
        /// </summary>
        /// <param name="FilterID">Filter Unique ID</param>
        /// <param name="FilterName"> Filter (Friendly) Name</param>
        public void AddFilter(int FilterID, string FilterName)
        {
            // Search collection for filter with current FilterID - if exists, just exit
            foreach (FilterItem element in  _event._FilterCollection)
                if (element.Id.Equals(FilterID)) return;

            // Insert new filter to collection
            FilterItem item = new FilterItem() { Id = FilterID, Name =  FilterName};
            _event._FilterCollection.Add(item);
        }

        public void SelFilter(int FilterId)
        {
            if (_event._FilterCollection != null)
            {
                foreach (FilterItem item in _event._FilterCollection)
                {
                    if (item.Id.Equals(FilterId))
                    {
                        _event.SelectedFilter = item;
                        return;
                    };
                }

                // Did not find filter
                SelFilter(-1);
            };
        }

#endregion Filter Interface

#region Attached Event
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




        /// Mouse entered a vJoy Input Channel
        /// Set HoveredvjInput to number of selected channel
        void NonRectangularWindow_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.DragMove();
        }

        private void LocChanged(object sender, System.EventArgs e)
        {
            if (OnMove != null)
                OnMove(Left, Top);
        }

        private void MouseEntervJoyInputCh(object sender, MouseEventArgs e)
        {
            WpfCtrlWin.ChannelCtrl cCtrl = sender as WpfCtrlWin.ChannelCtrl;
            if (cCtrl != null && cCtrl.IsEnabled && (e.LeftButton == MouseButtonState.Released))
                _event.HoveredvjInput = cCtrl.ChannelName;
            else
                _event.HoveredvjInput = "0";
        }

        /// Mouse Leaves a vJoy Input Channel
        /// Set HoveredvjInput to 0
        private void MouseLeavevJoyInputCh(object sender, MouseEventArgs e)
        {
            WpfCtrlWin.ChannelCtrl cCtrl = sender as WpfCtrlWin.ChannelCtrl;
            if (cCtrl != null && cCtrl.IsEnabled && (e.LeftButton == MouseButtonState.Released))
                _event.HoveredvjInput = "0";
        }


        private void MouseMovevJoyInputCh(object sender, MouseEventArgs e)
        {
            WpfCtrlWin.ChannelCtrl InCh2Drag = sender as WpfCtrlWin.ChannelCtrl;
            if (InCh2Drag != null && e.LeftButton == MouseButtonState.Pressed)
            {
                DragDrop.DoDragDrop(InCh2Drag, InCh2Drag.ChannelName, DragDropEffects.Copy);
            }
        }

        private void DragEnter_vJoyAxisRect(object sender, DragEventArgs e)
        {
        }

        private void Drop_vJoyAxisRect(object sender, DragEventArgs e)
        {

            WpfCtrlWin.ChannelCtrl TargetAxis = sender as WpfCtrlWin.ChannelCtrl;
            if (TargetAxis != null)
            {
                // If the DataObject contains string data, extract it. 
                if (e.Data.GetDataPresent(DataFormats.StringFormat))
                {
                    string SourceCh = (string)e.Data.GetData(DataFormats.StringFormat);
                    string TargetAx = TargetAxis.ChannelName;
                    int iSrc = 0;
                    bool converted = Int32.TryParse(SourceCh, out iSrc);
                    if (converted)
                        SendAxisMappingData(iSrc, TargetAx);
                }
            }
        }

        private void DragLeave_vJoyAxisRect(object sender, DragEventArgs e)
        {

        }

        private void DragOver_vJoyAxisRect(object sender, DragEventArgs e)
        {

        }


        // List of decoders - selection changed - Scroll into view
        private void Decoder_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListView list = e.OriginalSource as ListView;
            list.ScrollIntoView(list.SelectedItem);
            OnDecoderChanged((list.SelectedItem as DecoderItem).Name);
        }


        // Start Windows sound selector
        private void Audio_Click(object sender, RoutedEventArgs e)
        {
            //Process.Start("cmd", "/C control.exe mmsys.cpl,,1");
            Process.Start("control.exe", "mmsys.cpl,,1");
        }


    }
    
#endregion Attached Event

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

    /// <summary>
    /// Object of this class represents an array of level-monitors
    /// Used to display Axis-level and channel level
    /// To be replaced by an ObservableCollection of elements (ItemsControl) 
    /// </summary>
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

    public class BoolMonitor : LevelMonitor
    {
        private bool _level = false;
        private bool _enabled = true;

        public BoolMonitor(int id, String name) : base(id, name) { }

        // Accessors
        public new bool Level
        {
            get { return _level; }
            set { _level = value; }
        }

        public  bool Enabled
        {
            get { return _enabled; }
            set { _enabled = value; }
        }

        public new bool MapTarget(object target)
        {
            return false;
        }
    }

    /// <summary>
    /// Represents information about a constantly changing scalar
    /// such as a joystick axis or a data channel
    /// To support full versatility, it supports both mapping to and mapping by
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public class LevelMonitor
    {
        // Private (internal values)
        private uint _level = 50;       // Levels range 0-100
        private int _id = -1;           // Valid id is >=0
        private bool _implemented = false; // True if this axis/channel implemented
        private String _name = "";      // Friendly name for this object
        private List<object> _map_target = new List<object>();   // Array of objects to which this object is mapped to (For channels)
        private object _map_source;     // Object that mapped to this object

        // Constructors
        public LevelMonitor() { _map_target.Clear(); _map_source = ""; }
        public LevelMonitor(String name) { _map_target.Clear(); _map_source = ""; _name = name; }
        public LevelMonitor(int id) { _map_target.Clear(); _map_source = ""; _id = id; }
        public LevelMonitor(int id, String name) { _map_target.Clear(); _map_source = ""; _id = id; _name = name; }

        // Accessors
        public uint Level
        {
            get { return _level; }
            set
            {
                _level = value;
                if (_level > 100)
                    _level = 100;
            }
        }

        public object MapSource
        {
            get { return _map_source; }
            set 
            {
                if (!_map_source.Equals(value))
                {

                    _map_source = value; 
                };
            }
       }

        public bool Implemented
        {
            get { return _implemented; }
            set { _implemented = value;}
        }

        public int Id
        {
            get { return _id; }
            set { _id = value; if (_id < 1) _id = -1; }
        }

        public String Name
        {
            get { return _name; }
            set { _name = value; }
        }

        // Mapping interface
        /// <summary>
        /// Add an  object to the array of mapping targets
        /// If object exists do not add it - just return the index of the object
        /// </summary>
        /// <param name="target"> Add this object to the array of mapping targets</param>
        /// <returns>True if added or if object already exists - false otherwize</returns>
        public bool MapTarget(object target)
        {
            if (_map_target == null)
                return false;

            // Search for identical object
            foreach (object element in _map_target)
            {
                if (element.Equals(target)) return true;
            }

            // Not found - Add object
            _map_target.Add(target);
            return true;
        }

        /// <summary>
        /// Removes a mapping target from the list of targets
        /// </summary>
        /// <param name="target">Object to remove</param>
        /// <returns>true if removed</returns>
        public bool UnMapTarget(object target)
        {
            if (_map_target == null)
                return false;

            return _map_target.Remove(target);
        }

        public void UnMapAllTargets()
        {
            _map_target.Clear();
        }


        /// <summary>
        /// Gets the target object by index
        /// </summary>
        /// <param name="index">Index of object in the list of targets</param>
        /// <returns>The object (or null if out of range)</returns>
        public object GetMapTargetAt(uint index)
        {
            if (index > _map_target.Count)
                return null;

            return _map_target[(int)index];
        }
    }

    public class DecoderItem
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public string Subtype  { get; set; }
    }

    public class FilterItem
    {
        public string Name { get; set; }
        public int Id { get; set; }
    }

#endregion Data Structures

#region Converters

    /// Compare two string 
    /// If strings are identical (but not "0") then return Visibility.Visible
    /// Else return Visibility.Collapsed
    [ValueConversion(typeof(object), typeof(Visibility))]
    public class StringCompareConverter : IMultiValueConverter
    {
        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            string Val0;

            string param = parameter as string;
            if (param != null && param.Equals("Axis") && (values[0] is int))
            {
                int i = (int)values[0];
                if (i == 0 || values[0].Equals("0"))
                    return Visibility.Collapsed;
                Val0 = i.ToString();
            }
            else
                Val0 = (string)values[0];

            if (Val0 != null && !Val0.Equals("0"))
            {
                if (Val0.Equals(values[1]))
                    return Visibility.Visible;
                else
                    return Visibility.Collapsed;
            }

            return Visibility.Collapsed;
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            string[] output = {"Not",  "Implemented" };
            return output;
        }
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

    // Based on http://www.thomaslevesque.com/2011/03/21/wpf-how-to-bind-to-data-when-the-datacontext-is-not-inherited
    public class BindingProxy : Freezable
    {
        #region Overrides of Freezable

        protected override Freezable CreateInstanceCore()
        {
            return new BindingProxy();
        }

        #endregion

        public object Data
        {
            get { return (object)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        // Using a DependencyProperty as the backing store for Data.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DataProperty =
            DependencyProperty.Register("Data", typeof(object), typeof(BindingProxy), new UIPropertyMetadata(null));
    }

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