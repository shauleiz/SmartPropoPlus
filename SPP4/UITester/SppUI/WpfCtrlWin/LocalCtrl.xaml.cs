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
    /// Interaction logic for CannelCtrl.xaml
    /// </summary>
    public partial class ChannelCtrl : UserControl
    {
        public ChannelCtrl()
        {
            InitializeComponent();
        }

        public string ChannelName { set { ChNumTB.Content = value; } }
        public uint ChannelLevel { set { ChLevelPB.Value = value; } }
        public Brush BarColor { set { ChLevelPB.Foreground = value; } get { return ChLevelPB.Foreground; } }
        public Brush BarBgColor { set { ChLevelPB.Background = value; } get { return ChLevelPB.Background; } }          
    }
}
