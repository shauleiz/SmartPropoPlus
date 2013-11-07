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

namespace CtrlWindowNS
{
    public partial class CtrlWindow : Window
    {
        private EventModel _event;
        public delegate void ClockMoving(double Left, double Top);
        public event ClockMoving OnMove;

        public CtrlWindow()
        {
            InitializeComponent();

            // create a model object
            _event = new EventModel()
            {
                SelectedJack = "-- No audio jack Selected --"
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

        private void Button_Click(object sender, RoutedEventArgs e)
        {

            _event.SelectedJack = _event.SelectedJack.ToLower();
        }

        public void Set_TB_SelectedJack(string intext)
        {
            _event.SelectedJack = intext;
        }

        private void P1_Click(object sender, RoutedEventArgs e)
        {

        }

        private void P2_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
