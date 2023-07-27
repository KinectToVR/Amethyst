using System;
using Windows.Foundation;
using Amethyst.Classes;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;

namespace Amethyst.Installer.ViewModels;

public static class SetupData
{
    public static bool LimitedSetup { get; set; } = true;
    public static bool LimitedHide { get; set; } = true;
}

public class EnabledTemplateSelector : DataTemplateSelector
{
    public DataTemplate ItemTemplate { get; set; }

    protected override DataTemplate SelectTemplateCore(object item, DependencyObject container)
    {
        if (container is not SelectorItem selector || item is not SetupPlugin plugin) return ItemTemplate;
        var myBinding = new Binding
        {
            Source = item,
            Path = new PropertyPath(nameof(plugin.IsEnabled)),
            Mode = BindingMode.TwoWay,
            UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged
        };
        BindingOperations.SetBinding(selector, Control.IsEnabledProperty, myBinding);
        return ItemTemplate;
    }
}

public static class Extensions
{
    public static void ScrollToElement(this ScrollViewer scrollViewer, UIElement element,
        bool isVerticalScrolling = true, bool smoothScrolling = true, float? zoomFactor = null)
    {
        var transform = element.TransformToVisual((UIElement)scrollViewer.Content);
        var position = transform.TransformPoint(new Point(0, 0));

        if (isVerticalScrolling)
            scrollViewer.ChangeView(null, position.Y, zoomFactor, !smoothScrolling);
        else
            scrollViewer.ChangeView(position.X, null, zoomFactor, !smoothScrolling);
    }
}

public class InversionConverter : IValueConverter
{
    public object Convert(object value, Type targetType,
        object parameter, string language)
    {
        return !(value as bool? ?? false);
    }

    // ConvertBack is not implemented for a OneWay binding.
    public object ConvertBack(object value, Type targetType,
        object parameter, string language)
    {
        return null;
    }
}

public class InversionVisibilityConverter : IValueConverter
{
    public object Convert(object value, Type targetType,
        object parameter, string language)
    {
        return value as bool? ?? false
            ? Visibility.Collapsed
            : Visibility.Visible;
    }

    // ConvertBack is not implemented for a OneWay binding.
    public object ConvertBack(object value, Type targetType,
        object parameter, string language)
    {
        return null;
    }
}

public class InversionOpacityConverter : IValueConverter
{
    public object Convert(object value, Type targetType,
        object parameter, string language)
    {
        return value as bool? ?? false
            ? double.TryParse(
                parameter?.ToString(), out var result)
                ? result
                : 0.5
            : 1.0;
    }

    // ConvertBack is not implemented for a OneWay binding.
    public object ConvertBack(object value, Type targetType,
        object parameter, string language)
    {
        return null;
    }
}

public class OpacityConverter : IValueConverter
{
    public object Convert(object value, Type targetType,
        object parameter, string language)
    {
        return value as bool? ?? false ? 1.0 : 0.0;
    }

    // ConvertBack is not implemented for a OneWay binding.
    public object ConvertBack(object value, Type targetType,
        object parameter, string language)
    {
        return null;
    }
}