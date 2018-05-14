using System.Runtime.InteropServices;

namespace PowerKraut_Core.vulkan{
    internal static class KrautVK{
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Init(int width, int height, string title, bool fullscreen);
         
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool WindowShouldClose();
        
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void PollEvents();
        
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Terminate();
    }
}