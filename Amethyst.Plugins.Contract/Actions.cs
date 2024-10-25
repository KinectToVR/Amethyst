using System;
using System.Diagnostics.CodeAnalysis;

namespace Amethyst.Plugins.Contract;

// Input action declaration for key events
public interface IKeyInputAction : IComparable<IKeyInputAction>, IEquatable<IKeyInputAction>
{
    /// <summary>
    ///     Identifies the action
    /// </summary>
    public string Guid { get; init; }

    /// <summary>
    ///     Friendly name of the action
    /// </summary>
    public string Name { get; set; }

    /// <summary>
    ///     Action description for binding UI
    /// </summary>
    public string Description { get; set; }

    /// <summary>
    ///     Action image for binding UI to be shown in info
    ///     MUST BE OF TYPE Microsoft.UI.Xaml.Controls.Image
    /// </summary>
    /// <remarks>
    ///     Make this a getter and return an Image constructed
    ///     during OnLoad, expect COM-related crashes otherwise
    /// </remarks>
    public object? Image { get; set; }

    /// <summary>
    ///     Invoke the action (shortcut)
    /// </summary>
    public Action<object?> Invoke => _ => { };

    /// <summary>
    ///     Checks whether the action is used for anything
    /// </summary>
    public bool IsUsed => false;

    /// <summary>
    ///     Action data type (shortcut)
    /// </summary>
    public Type DataType => typeof(object);
}

// Input action declaration for key events
public class KeyInputAction<T> : IKeyInputAction
{
    /// <summary>
    ///     Identifies the action
    /// </summary>
    public string Guid { get; init; } = System.Guid.NewGuid().ToString();

    /// <summary>
    ///     Friendly name of the action
    /// </summary>
    public string Name { get; set; } = "INVALID";

    /// <summary>
    ///     Action description for binding UI
    /// </summary>
    public string Description { get; set; } = string.Empty;

    /// <summary>
    ///     Action image for binding UI to be shown in info
    ///     MUST BE OF TYPE Microsoft.UI.Xaml.Controls.Image
    /// </summary>
    /// <remarks>
    ///     Make this a getter and return an Image constructed
    ///     during OnLoad, expect COM-related crashes otherwise
    /// </remarks>
    public object? Image { get; set; }

    /// <summary>
    ///     Implement comparator with other actions (by Guid)
    /// </summary>
    public int CompareTo(IKeyInputAction? other)
    {
        return string.Compare(Guid, other?.Guid, StringComparison.Ordinal);
    }

    /// <summary>
    ///     Implement comparator with other actions (by Guid)
    /// </summary>
    public bool Equals(IKeyInputAction? other)
    {
        return Guid.Equals(other?.Guid);
    }

    /// <summary>
    ///     Implement comparator with other objects
    /// </summary>
    public override bool Equals(object? obj)
    {
        return Equals(obj as IKeyInputAction);
    }

    /// <summary>
    ///     Implement hashes for the comparator
    /// </summary>
    public override int GetHashCode()
    {
        return Guid.GetHashCode();
    }

    /// <summary>
    ///     Host import for Invoke() calls and stuff
    ///     Func so you can use it in static context 
    /// </summary>
    public Func<IAmethystHost?> GetHost { get; set; } = () => null;

    /// <summary>
    ///     Invoke the action (shortcut)
    /// </summary>
    public Action<T?> Invoke => data => GetHost()?.ReceiveKeyInput(this, data);

    /// <summary>
    ///     Checks whether the action is used for anything
    /// </summary>
    public bool IsUsed => GetHost()?.CheckInputActionIsUsed(this) ?? false;

    /// <summary>
    ///     Action data type (shortcut)
    /// </summary>
    public Type DataType => typeof(T);
}