using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WpfCtrlWin
{
    /// <summary>
    /// Interaction logic for JoyButton.xaml
    /// </summary>
    public partial class JoyButton : UserControl
    {
        public JoyButton()
        {
            InitializeComponent();
        }

        // Defaults
        private bool _pressed = false;
        private bool _mapped = false;
        private bool _enabled = false;
        private int _BtnValue = 0;
        private int _BtnLast = 0;
        private Brush _bgPressedBrush = new SolidColorBrush(Colors.Red);
        private Brush _bgReleasedBrush = new SolidColorBrush(Colors.LightGray);
        private Brush _bgUnMappedBrush = new SolidColorBrush(Colors.Wheat);
        private Brush _bgDisabledBrush = new SolidColorBrush(Colors.Yellow);
        private Brush _ringStrokeBrush = new SolidColorBrush(Colors.Black);
        private Brush _ringDisabledBrush = new SolidColorBrush(Colors.Transparent);

        // Override defaults
        public Brush BgPressedBrush {set { _bgPressedBrush = value; } get { return _bgPressedBrush;}}
        public Brush BgReleasedBrush { set { _bgReleasedBrush = value; } get { return _bgReleasedBrush;}}
        public Brush BgUnMappedBrush { set { _bgUnMappedBrush = value; } get { return _bgUnMappedBrush; } }
        public Brush BgDisabledBrush { set { _bgDisabledBrush = value; } get { return _bgDisabledBrush; } }

        // Internal properties

        /// <summary>
        /// Compute value of background brush
        /// </summary>
        private Brush BgColor() 
        { 
            if (_BtnLast < _BtnValue)
                _enabled = false;
            else
                _enabled = true;

                if (!_enabled)
                    BackGround.Fill = _bgDisabledBrush;
                else
                {
                    if (!_mapped)
                        BackGround.Fill = _bgUnMappedBrush;
                    else
                    {
                        if (_pressed)
                            BackGround.Fill = _bgPressedBrush;
                        else
                            BackGround.Fill = _bgReleasedBrush;
                    }
                }

                return BackGround.Fill;
        }


        private Brush RingStroke { set { OuterRing.Stroke = value; } get { return OuterRing.Stroke; } }



#if false

        /// <summary>
        /// IsEnabled - Represents an existing vJoy device button
        /// Value is changed when device changes
        /// Other properties ('Mapped', 'Pressed') are meaningful only when button is enabled
        /// </summary>
        public new bool IsEnabled
        {
            get { return (bool)GetValue(IsEnabledProperty); }
            set { SetValue(IsEnabledProperty, value); }
        }

        public new static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.Register("IsEnabled", typeof(bool), typeof(JoyButton), new UIPropertyMetadata(false, (o, e) =>
            {
                ((JoyButton)o)._enabled = (bool)e.NewValue;
                ((JoyButton)o).BackGround.Fill = ((JoyButton)o).BgColor();
            }));
        
#endif
        /// <summary>
        /// IsMapped - True if an input channel is mapped to an enabled vJoy device button
        /// Property 'Pressed' is meaningful only when button is Mapped
        /// </summary>
        public  bool IsMapped
        {
            get { return (bool)GetValue(IsMappedProperty); }
            set { SetValue(IsMappedProperty, value); }
        }

        public  static readonly DependencyProperty IsMappedProperty =
            DependencyProperty.Register("IsMapped", typeof(bool), typeof(JoyButton), new UIPropertyMetadata(false, (o, e) =>
            {
                ((JoyButton)o)._mapped = (bool)e.NewValue;
                ((JoyButton)o).BackGround.Fill = ((JoyButton)o).BgColor(); 
            }));

        /// <summary>
        /// IsPressed - True if an input button is 'Pressed' (Only Mapped buttons can be 'Pressed')
        /// </summary>
        public  bool IsPressed
        {
            get { return (bool)GetValue(IsPressedProperty); }
            set { SetValue(IsPressedProperty, value); }
        }

        public  static readonly DependencyProperty IsPressedProperty =
            DependencyProperty.Register("IsPressed", typeof(bool), typeof(JoyButton), new UIPropertyMetadata(false, (o, e) =>
            {
                ((JoyButton)o)._pressed = (bool)e.NewValue;
                ((JoyButton)o).BackGround.Fill = ((JoyButton)o).BgColor(); 
            }));

        /// <summary>
        /// LastButton - The number of enabled vJoy device buttons - Used to determened if a button enabled
        /// Button is enabled if its number is leaa or equal to the 'LastButton'
        /// </summary>
        public int LastButton
        {
            get { return (int)GetValue(LastButtonProperty); }
            set { SetValue(LastButtonProperty, value); }
        }

        public static readonly DependencyProperty LastButtonProperty =
            DependencyProperty.Register("LastButton", typeof(int), typeof(JoyButton), new UIPropertyMetadata(0, (o, e) =>
            {
             //   if (int.TryParse(((JoyButton)o).Number, out result))
                ((JoyButton)o)._BtnLast = (int)e.NewValue;
             //   else
              //      ((JoyButton)o)._enabled = false;

                ((JoyButton)o).BackGround.Fill = ((JoyButton)o).BgColor();
            }));

        public int Number
        {
            get { return (int)GetValue(NumberProperty); }
            set { SetValue(NumberProperty, value); }
        }

        public static readonly DependencyProperty NumberProperty =
            DependencyProperty.Register("Number", typeof(int), typeof(JoyButton), new UIPropertyMetadata(0, (o, e) =>
            {
                ((JoyButton)o)._BtnValue =  (int)e.NewValue;
                ((JoyButton)o).ButtonNumber.Content = ((JoyButton)o)._BtnValue;
                ((JoyButton)o).BackGround.Fill = ((JoyButton)o).BgColor();
            }));

    }
}
