using System;
using System.Reflection;
using System.Runtime.Loader;

namespace Amethyst.MVVM;

public class ModuleAssemblyLoadContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;

    internal ModuleAssemblyLoadContext(string assemblyPath) : base(false)
    {
        _resolver = new AssemblyDependencyResolver(assemblyPath);

        ResolvingUnmanagedDll += OnResolvingUnmanaged;
        Resolving += OnResolving;
    }

    private IntPtr OnResolvingUnmanaged(Assembly assembly, string unmanagedName)
    {
        var unmanagedPath = _resolver.ResolveUnmanagedDllToPath(unmanagedName);
        return unmanagedPath != null ? LoadUnmanagedDllFromPath(unmanagedPath) : IntPtr.Zero;
    }

    private Assembly OnResolving(AssemblyLoadContext context, AssemblyName assemblyName)
    {
        var assemblyPath = _resolver.ResolveAssemblyToPath(assemblyName);
        return assemblyPath != null ? LoadFromAssemblyPath(assemblyPath) : null;
    }
}