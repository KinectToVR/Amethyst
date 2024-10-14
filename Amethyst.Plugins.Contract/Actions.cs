using System;
using System.Diagnostics.CodeAnalysis;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace Amethyst.Plugins.Contract;

// Input action declaration for key events
public abstract class KeyInputAction : IComparable<KeyInputAction>, IEquatable<KeyInputAction>
{
    /// <summary>
    ///     Identifies the action
    /// </summary>
    public Guid Guid { get; init; } = Guid.Empty;

    /// <summary>
    ///     Friendly name of the action
    /// </summary>
    public string Name { get; set; } = string.Empty;

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
    ///     Invoke the action (shortcut)
    /// </summary>
    public Action<object?> Invoke => _ => { };

    /// <summary>
    ///     Action data type (shortcut)
    /// </summary>
    public Type DataType => typeof(object);

    /// <summary>
    ///     Implement comparator with other actions (by Guid)
    /// </summary>
    public int CompareTo(KeyInputAction? other)
    {
        return Guid.CompareTo(other?.Guid);
    }

    /// <summary>
    ///     Implement comparator with other actions (by Guid)
    /// </summary>
    public bool Equals(KeyInputAction? other)
    {
        return Guid.Equals(other?.Guid);
    }

    /// <summary>
    ///     Implement comparator with other objects
    /// </summary>
    public override bool Equals(object? obj)
    {
        return Equals(obj as KeyInputAction);
    }

    /// <summary>
    ///     Implement hashes for the comparator
    /// </summary>
    public override int GetHashCode()
    {
        return Guid.GetHashCode();
    }
}

// Input action declaration for key events
public class KeyInputAction<T> : KeyInputAction
{
    [SetsRequiredMembers]
    public KeyInputAction()
    {
        Guid = Guid.NewGuid();
        Name = "INVALID";
    }

    /// <summary>
    ///     Host import for Invoke() calls
    /// </summary>
    public IAmethystHost? Host { get; set; }

    /// <summary>
    ///     Invoke the action (shortcut)
    /// </summary>
    public new Action<T?> Invoke => data => Host?.ReceiveKeyInput(this, data);

    /// <summary>
    ///     Action data type (shortcut)
    /// </summary>
    public new Type DataType => typeof(T);
}