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

#if false
        public  bool IsEnabled1
        {
            get { return (bool)GetValue(IsEnabled1Property); }
            set { SetValue(IsEnabled1Property, value); }
        }

        public  readonly DependencyProperty IsEnabled1Property =
            DependencyProperty.Register("IsEnabled1", typeof(bool), typeof(ChannelCtrl), new UIPropertyMetadata(true, (o, e) =>
            {
                ((ChannelCtrl)o).ChannelLevel = (bool)e.NewValue ? (uint)10 : (uint)60;
                ((ChannelCtrl)o).ChLevelPB.IsEnabled = (bool)e.NewValue;
                ((ChannelCtrl)o).IsEnabled = ((ChannelCtrl)o).IsEnabled1;
            }));
        
#endif
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
                //_barbgcolor = ChLevelPB.Background.ToString();
                ChLevelPB.Background = value;
            }
            get { return ChLevelPB.Background; }
        }

#if false
        public bool IsEnabled
        {
            get { return (bool)GetValue(IsEnabledProperty); }
            set { SetValue(IsEnabledProperty, value); }
        }
        public static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.Register("IsEnabled", typeof(bool), typeof(ChannelCtrl), new UIPropertyMetadata(false, (o, e) =>
            {
                ((ChannelCtrl)o).ChLevelPB.Foreground = new SolidColorBrush(Colors.LightGray);
            }));
        
#endif    
    }
}
