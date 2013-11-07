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
 //               Date = new DateTime(2011, 7, 1),
                SelectedJack = "-- No audio jack Selected --"
            };

            // bind the Date to the UI
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
 //           _event.Date = _event.Date.AddDays(1);
        }

        public void Set_TB_SelectedJack(string intext)
        {
            _event.SelectedJack = intext;
        }
    }
}
