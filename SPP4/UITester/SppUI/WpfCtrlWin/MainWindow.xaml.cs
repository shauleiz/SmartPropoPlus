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
using System.Globalization;
using System.Diagnostics;
using System.Text;

namespace CtrlWindowNS
{
    public partial class CtrlWindow : Window
    {
        private EventModel _event;
        public delegate void ClockMoving(double Left, double Top);
        public event ClockMoving OnMove;

        public CtrlWindow()
        {
            BindingErrorTraceListener.SetTrace();
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
   
}

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