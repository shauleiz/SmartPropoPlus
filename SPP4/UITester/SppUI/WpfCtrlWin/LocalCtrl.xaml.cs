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

        public Brush BarColorSv;
        public Brush BarBgColorSv;
        public UInt32 ChannelLevelSv = 50;

        public string ChannelName { set { ChNumTB.Content = value; } }


        public UInt32 ChannelLevel { 
            set { SetValue(ChannelLevelProperty, value); }
            get { return (UInt32)GetValue(ChannelLevelProperty); }
        }  

        public static readonly DependencyProperty ChannelLevelProperty =
            DependencyProperty.Register("ChannelLevel", typeof(UInt32), typeof(ChannelCtrl), new UIPropertyMetadata((UInt32)0, (o, e) =>
            {
                ((ChannelCtrl)o).ChLevelPB.Value = (UInt32)e.NewValue;
                ((ChannelCtrl)o).ChannelLevelSv = (UInt32)e.NewValue;
            }));


        public Brush BarColor
        {
            set { SetValue(BarColorProperty, value); }
            get { return (Brush)GetValue(BarColorProperty); }
        }  

        public static readonly DependencyProperty BarColorProperty =
            DependencyProperty.Register("BarColor", typeof(Brush), typeof(ChannelCtrl), new UIPropertyMetadata((Brush)null, (o, e) =>
        { 
                ((ChannelCtrl)o).BarColorSv = (Brush)e.NewValue;
                if (((ChannelCtrl)o).IsEnabled)
                    ((ChannelCtrl)o).ChLevelPB.Foreground = (Brush)e.NewValue;
                else
                    ((ChannelCtrl)o).ChLevelPB.Foreground = new SolidColorBrush(Colors.Transparent);

        } ));

        public Brush BarBgColor
        {
            set
            {
                BarBgColorSv = value;
                if (IsEnabled)
                    ChLevelPB.Background = value;
                else
                    ChLevelPB.Background = new SolidColorBrush(Colors.Transparent);
            }
            get { return ChLevelPB.Background; }
        }
        
        public new bool IsEnabled
        {
            get { return (bool)GetValue(IsEnabledProperty); }
            set { SetValue(IsEnabledProperty, value); }
        }

        public new static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.Register("IsEnabled", typeof(bool), typeof(ChannelCtrl), new UIPropertyMetadata(false, (o, e) =>
            {
                if (!(bool)e.NewValue)
                {
                    ((ChannelCtrl)o).ChannelLevel = 0;
                    ((ChannelCtrl)o).ChLevelPB.Foreground = new SolidColorBrush(Colors.Transparent);
                    ((ChannelCtrl)o).ChLevelPB.Background = new SolidColorBrush(Colors.Transparent);
                }
                else
                {
                    ((ChannelCtrl)o).BarColor = ((ChannelCtrl)o).BarColorSv.CloneCurrentValue();
                    ((ChannelCtrl)o).BarBgColor = ((ChannelCtrl)o).BarBgColorSv.CloneCurrentValue();
                    ((ChannelCtrl)o).ChLevelPB.Value = ((ChannelCtrl)o).ChannelLevelSv;
                }
            }));


    }
}
