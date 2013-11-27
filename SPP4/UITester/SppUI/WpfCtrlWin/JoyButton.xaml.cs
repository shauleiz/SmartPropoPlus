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

        private bool _pressed = false;
        private bool _mapped = false;
       private Brush _bgPressedBrush = new SolidColorBrush(Colors.Red);
        private Brush _bgReleasedBrush = new SolidColorBrush(Colors.LightGray);
        private Brush _bgUnMappedBrush = new SolidColorBrush(Colors.Transparent);

        public Brush BgColor { set { BackGround.Fill = value; } get { return BackGround.Fill; } }
        public String Number { set { ButtonNumber.Content = value; } get { return ButtonNumber.Content as string; } }
        public bool Pressed
        {
            set {
                _pressed = value;
                if (_pressed)
                    BgColor = _bgPressedBrush;
                else
                    BgColor = _bgReleasedBrush;
            }
            get { return _pressed;}
        } //  Pressed


        public bool Mapped
        {
            set
            {
                _mapped = value;
                if (_mapped)
                    Pressed = _pressed;
                else
                    BgColor = _bgUnMappedBrush;
            }
            get { return _mapped; }
        } // Mapped

    }
}
