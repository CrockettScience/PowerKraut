using System.Runtime.InteropServices;

namespace PowerKraut_Core.vulkan{
    internal static class KrautVK{
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Init();
         
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool WindowShouldClose();
        
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int PollEvents();
    }
}