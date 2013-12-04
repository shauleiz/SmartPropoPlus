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

        public Brush BarColorSv = null;
        public uint ChannelLevelSv = 50;


        public string ChannelName { set { ChNumTB.Content = value; } }
        public uint ChannelLevel { set { ChLevelPB.Value = value; } }
        public Brush BarColor { 
            set 
            {
                if (IsEnabled)
                    ChLevelPB.Foreground = value;
                else
                    ChLevelPB.Foreground = new SolidColorBrush(Colors.LightGray);

            } 
            get { return ChLevelPB.Foreground; } }
        public Brush BarBgColor
        {
            set
            {
                ChLevelPB.Background = value;
            }
            get { return ChLevelPB.Background; }
        }

        public new bool IsEnabled
        {
            get { return (bool)GetValue(IsEnabledProperty); }
            set { SetValue(IsEnabledProperty, value); }
        }
        public new static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.Register("IsEnabled", typeof(bool), typeof(ChannelCtrl), new UIPropertyMetadata(true, (o, e) =>
            {
                if (!(bool)e.NewValue)
                {
                    ((ChannelCtrl)o).ChannelLevelSv = (uint)((ChannelCtrl)o).ChLevelPB.Value;
                    ((ChannelCtrl)o).ChannelLevel = 0;
                    ((ChannelCtrl)o).BarColorSv = ((ChannelCtrl)o).BarColor.CloneCurrentValue();
                    ((ChannelCtrl)o).ChLevelPB.Foreground = new SolidColorBrush(Colors.Transparent);
                }
                else
                {
                    ((ChannelCtrl)o).BarColor =  ((ChannelCtrl)o).BarColorSv.CloneCurrentValue();
                    ((ChannelCtrl)o).ChLevelPB.Value = ((ChannelCtrl)o).ChannelLevelSv;
                }
            }));


    }
}
