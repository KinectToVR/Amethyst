using Microsoft.UI.Xaml;

namespace K2CrashHandler.Triggers;

public class VisibilityTrigger : StateTriggerBase
{
    private FrameworkElement _element;
    private Visibility _trigger;

    public FrameworkElement Target
    {
        get => _element;
        set
        {
            _element = value;
            RefreshState();
        }
    }

    public Visibility ActiveOn
    {
        get => _trigger;
        set
        {
            _trigger = value;
            RefreshState();
        }
    }

    private void RefreshState()
    {
        SetActive(Target?.Visibility == ActiveOn);
    }
}