using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;

namespace LibMMD;

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct Vec4f
{
    public float X = 0f;
    public float Y = 0f;
    public float Z = 0f;
    public float W = 1f;

    public Vec4f(in float x, in float y, in float z, in float w)
    {
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    public Vec4f(in Quaternion q)
    {
        X = q.X;
        Y = q.Y;
        Z = q.Z;
        W = q.W;
    }

    public Quaternion Vec()
    {
        return new Quaternion(X, Y, Z, W);
    }
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct Vec3f
{
    public float X = 0f;
    public float Y = 0f;
    public float Z = 0f;

    public Vec3f(in float x, in float y, in float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    public Vec3f(in Vector3 v)
    {
        X = v.X;
        Y = v.Y;
        Z = v.Z;
    }

    public Vector3 Vec()
    {
        return new Vector3(X, Y, Z);
    }
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct Vec2f
{
    public float X = 0f;
    public float Y = 1f;

    public Vec2f(in float x, in float y)
    {
        X = x;
        Y = y;
    }
}

public class Vmd
{
    public int FrameIndex { get; set; } = 0;
    public int TrackersFrameIndex { get; set; } = 0;

    public string Magic { get; internal set; } = "Vocaloid Motion Data 0002";
    public string ModelName { get; internal set; } = "DIVA風ミク";
    public List<VmdBoneKeyFrame> BoneKeyFrames { get; internal set; } = [];
    public List<VmdMorphKeyFrame> MorphKeyFrames { get; internal set; } = [];
    public List<VmdCameraKeyFrame> CameraKeyFrames { get; internal set; } = [];
    public List<VmdLightKeyFrame> LightKeyFrames { get; internal set; }
    public List<VmdShadowKeyFrame> ShadowKeyFrames { get; internal set; }
    public List<VmdInverseKinematicKeyFrame> InverseKinematicKeyFrames { get; internal set; }

    internal Vmd()
    {
    }
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public unsafe struct VmdBoneKeyFrame
{
    private static readonly Encoding ShiftJis = Encoding.GetEncoding("shift-jis");
    private const int NAME_BYTES = 15;

    private fixed byte NameBytes[NAME_BYTES];
    public int FrameIndex;
    public Vec3f Position;
    public Vec4f RotationQuaternion;
    public fixed byte InterpolationData[64];

    public string Name
    {
        get
        {
            fixed (byte* p = NameBytes)
            {
                return ShiftJis.GetString(p, NAME_BYTES).Split('\0')[0];
            }
        }
        set
        {
            var encodedBytes = ShiftJis.GetBytes(value);
            for (var i = 0; i < NAME_BYTES; i++)
                NameBytes[i] = 0;
            for (var i = 0; i < encodedBytes.Length; i++)
                NameBytes[i] = encodedBytes[i];
        }
    }
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public unsafe struct VmdMorphKeyFrame
{
    private static readonly Encoding ShiftJis = Encoding.GetEncoding("shift-jis");
    private const int NAME_BYTES = 15;

    private fixed byte NameBytes[NAME_BYTES];
    public int FrameIndex;
    public float Weight;

    public string Name
    {
        get
        {
            fixed (byte* p = NameBytes)
            {
                return ShiftJis.GetString(p, NAME_BYTES).Split('\0')[0];
            }
        }
    }
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public unsafe struct VmdCameraKeyFrame
{
    public int FrameIndex;
    public float Length;
    public Vec3f Position;
    public Vec3f Rotation;
    public fixed byte InterpolationData[24];
    public int FoVAngle;
    public byte IsPerspectiveCamera;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public unsafe struct VmdLightKeyFrame
{
    public int FrameIndex;
    public Vec3f Color;
    public Vec3f Location;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public unsafe struct VmdShadowKeyFrame
{
    public int FrameIndex;
    public byte Mode;
    public float Distance;
}

public struct VmdInverseKinematicKeyFrame
{
    public int FrameIndex;
    public byte Show;
    public List<VmdInverseKinematicInfo> IkInfos;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public unsafe struct VmdInverseKinematicInfo
{
    private static readonly Encoding ShiftJis = Encoding.GetEncoding("shift-jis");
    private const int NAME_BYTES = 20;

    private fixed byte NameBytes[NAME_BYTES];
    public byte Enabled;

    public string Name
    {
        get
        {
            fixed (byte* p = NameBytes)
            {
                return ShiftJis.GetString(p, NAME_BYTES).Split('\0')[0];
            }
        }
    }
}

public class VmdParser
{
    public static Vmd Parse(Stream stream)
    {
        using var reader = new BinaryReader(stream, Encoding.Default, true);
        return Parse(reader);
    }

    public static Vmd Parse(BinaryReader reader)
    {
        var encoding = Encoding.GetEncoding("shift-jis");
        var vmd = new Vmd
        {
            Magic = reader.ReadFSString(30, encoding)
        };

        if (vmd.Magic.StartsWith("Vocaloid Motion Data file"))
            vmd.ModelName = reader.ReadFSString(10, encoding);
        else if (vmd.Magic.StartsWith("Vocaloid Motion Data 0002"))
            vmd.ModelName = reader.ReadFSString(20, encoding);
        else
            return null;

        vmd.BoneKeyFrames = reader.ReadArray<VmdBoneKeyFrame>(reader.ReadInt32());
        vmd.MorphKeyFrames = []; // reader.ReadArray<VmdMorphKeyFrame>(reader.ReadInt32());
        vmd.CameraKeyFrames = []; // reader.ReadArray<VmdCameraKeyFrame>(reader.ReadInt32());

        return vmd;

        /*

        if (!reader.IsEndOfStream())
            vmd.LightKeyFrames = reader.ReadArray<VmdLightKeyFrame>(reader.ReadInt32());

        if (!reader.IsEndOfStream())
            vmd.ShadowKeyFrames = reader.ReadArray<VmdShadowKeyFrame>(reader.ReadInt32());

        if (reader.IsEndOfStream()) return vmd;

        var ikCount = reader.ReadInt32();
        vmd.InverseKinematicKeyFrames = Enumerable.Range(0, ikCount).Select(i =>
            new VmdInverseKinematicKeyFrame()
            {
                FrameIndex = reader.ReadInt32(),
                Show = reader.ReadByte(),
                IkInfos = reader.ReadArray<VmdInverseKinematicInfo>(reader.ReadInt32())
            }).ToList();

        return vmd;

        */
    }

    public static void Save(Vmd vmd, string output)
    {
        var encoding = Encoding.GetEncoding("shift-jis");
        using var writer = new BinaryWriter(
            File.Open(output, FileMode.OpenOrCreate), encoding);

        writer.WriteFSString(vmd.Magic, 30, encoding);

        if (vmd.Magic.StartsWith("Vocaloid Motion Data file"))
            writer.WriteFSString(vmd.ModelName, 10, encoding);
        else if (vmd.Magic.StartsWith("Vocaloid Motion Data 0002"))
            writer.WriteFSString(vmd.ModelName, 20, encoding);

        writer.Write(vmd.BoneKeyFrames.Count);
        writer.WriteArray(vmd.BoneKeyFrames);

        writer.Write(vmd.MorphKeyFrames.Count);
        writer.WriteArray(vmd.MorphKeyFrames);

        writer.Write(vmd.CameraKeyFrames.Count);
        writer.WriteArray(vmd.CameraKeyFrames);

        //if (vmd.LightKeyFrames.Any())
        //{
        //    writer.Write(vmd.LightKeyFrames.Count);
        //    writer.WriteArray(vmd.LightKeyFrames);
        //}

        //if (vmd.ShadowKeyFrames.Any())
        //{
        //    writer.Write(vmd.ShadowKeyFrames.Count);
        //    writer.WriteArray(vmd.ShadowKeyFrames);
        //}

        //if (!(vmd.InverseKinematicKeyFrames?.Any() ?? false)) return;
        //writer.Write(vmd.InverseKinematicKeyFrames.Count);
        //foreach (var frame in vmd.InverseKinematicKeyFrames)
        //{
        //    writer.Write(frame.FrameIndex);
        //    writer.Write(frame.Show);
        //    writer.Write(frame.IkInfos.Count);
        //    writer.WriteArray(frame.IkInfos);
        //}
    }
}

internal static class Extensions
{
    public static T[] Extend<T>(this T[] array, int size)
    {
        if (array.Length < size)
            Array.Resize(ref array, size);
        return array;
    }

    public static unsafe T ReadStruct<T>(this BinaryReader reader) where T : unmanaged
    {
        var buffer = reader.ReadBytes(sizeof(T));
        fixed (byte* p = buffer)
        {
            return *(T*)p;
        }
    }

    public static unsafe List<T> ReadArray<T>(this BinaryReader reader, int elementCount) where T : unmanaged
    {
        if (elementCount <= 0) return [];

        var array = new T[elementCount];
        var buffer = reader.ReadBytes(sizeof(T) * elementCount);
        fixed (byte* src = buffer)
        {
            fixed (T* dst = array)
            {
                Buffer.MemoryCopy(src, dst, buffer.Length, buffer.Length);
            }
        }

        return array.ToList();
    }

    public static unsafe T ReadEnum<T>(this BinaryReader reader) where T : unmanaged, Enum
    {
        var size = sizeof(T);
        return size switch
        {
            1 => (T)(object)reader.ReadByte(),
            2 => (T)(object)reader.ReadInt16(),
            4 => (T)(object)reader.ReadInt32(),
            _ => default
        };
    }

    public static int ReadVarInt(this BinaryReader reader, int size, bool unsigned = false)
    {
        return size switch
        {
            1 => unsigned ? (int)reader.ReadByte() : reader.ReadSByte(),
            2 => unsigned ? (int)reader.ReadUInt16() : reader.ReadInt16(),
            4 => reader.ReadInt32(),
            _ => default
        };
    }

    /// <summary>
    /// Reads a length prefixed string (4 bytes) from the binary reader
    /// </summary>
    /// <param name="reader"></param>
    /// <param name="encoding"></param>
    /// <returns></returns>
    public static string ReadLPString(this BinaryReader reader, Encoding encoding)
    {
        if (reader.BaseStream.Length - reader.BaseStream.Position < 4) return default;
        var len = reader.ReadInt32();
        var remainingBytes = reader.BaseStream.Length - reader.BaseStream.Position;
        if (len < 0 || len > remainingBytes) return default;
        var stringBytes = reader.ReadBytes(len);
        return len > 0 ? encoding.GetString(stringBytes) : "";
    }

    /// <summary>
    /// Reads a string with a fixed size length
    /// </summary>
    /// <param name="reader"></param>
    /// <param name="maxLen"></param>
    /// <param name="encoding"></param>
    /// <returns></returns>
    public static string ReadFSString(this BinaryReader reader, int maxLen, Encoding encoding)
    {
        if (reader.BaseStream.Length - reader.BaseStream.Position < maxLen) return default;
        var stringBytes = reader.ReadBytes(maxLen);
        return encoding.GetString(stringBytes).Split('\0')[0];
    }

    public static bool IsEndOfStream(this BinaryReader reader)
    {
        return reader.BaseStream.Position >= reader.BaseStream.Length;
    }

    public static unsafe void WriteStruct<T>(this BinaryWriter writer, T value) where T : unmanaged
    {
        var size = sizeof(T);
        var buffer = new byte[size];
        fixed (byte* ptr = buffer)
        {
            *(T*)ptr = value;
        }

        writer.Write(buffer);
    }

    public static unsafe void WriteArray<T>(this BinaryWriter writer, List<T> array) where T : unmanaged
    {
        if (array == null || array.Count == 0) return;

        var size = sizeof(T) * array.Count;
        var buffer = new byte[size];
        fixed (T* src = array.ToArray())
        fixed (byte* dst = buffer)
        {
            Buffer.MemoryCopy(src, dst, size, size);
        }

        writer.Write(buffer);
    }

    public static unsafe void WriteEnum<T>(this BinaryWriter writer, T value) where T : unmanaged, Enum
    {
        var size = sizeof(T);
        switch (size)
        {
            case 1:
                writer.Write((byte)(object)value);
                break;
            case 2:
                writer.Write((short)(object)value);
                break;
            case 4:
                writer.Write((int)(object)value);
                break;
            default:
                return;
        }
    }

    public static void WriteVarInt(this BinaryWriter writer, int value, int size, bool unsigned = false)
    {
        switch (size)
        {
            case 1:
                if (unsigned)
                    writer.Write((byte)value);
                else
                    writer.Write((sbyte)value);
                break;
            case 2:
                if (unsigned)
                    writer.Write((ushort)value);
                else
                    writer.Write((short)value);
                break;
            case 4:
                writer.Write(value);
                break;
            default:
                return;
        }
    }

    /// <summary>
    /// Writes a length-prefixed string (4 bytes prefix) to the binary writer.
    /// </summary>
    /// <param name="writer"></param>
    /// <param name="value"></param>
    /// <param name="encoding"></param>
    public static void WriteLPString(this BinaryWriter writer, string value, Encoding encoding)
    {
        if (value == null)
        {
            writer.Write(0);
            return;
        }

        var stringBytes = encoding.GetBytes(value);
        writer.Write(stringBytes.Length);
        writer.Write(stringBytes);
    }

    /// <summary>
    /// Writes a string with a fixed size length to the binary writer, padding with null characters if necessary.
    /// </summary>
    /// <param name="writer"></param>
    /// <param name="value"></param>
    /// <param name="maxLen"></param>
    /// <param name="encoding"></param>
    public static void WriteFSString(this BinaryWriter writer, string value, int maxLen, Encoding encoding)
    {
        value ??= string.Empty;

        var stringBytes = encoding.GetBytes(value);
        if (stringBytes.Length > maxLen) return;

        writer.Write(stringBytes);
        if (stringBytes.Length < maxLen) writer.Write(new byte[maxLen - stringBytes.Length]);
    }
}