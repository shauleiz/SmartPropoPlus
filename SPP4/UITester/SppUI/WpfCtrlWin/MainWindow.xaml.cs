using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class CtrlWindow : Window
    {
        public delegate void ClockMoving(double Left, double Top);
        public event ClockMoving OnMove;

        public CtrlWindow()
        {
            InitializeComponent();
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
    }
}
