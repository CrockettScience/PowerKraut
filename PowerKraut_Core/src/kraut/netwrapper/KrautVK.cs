using System.Runtime.InteropServices;
using PowerKraut_Core.kraut.util.exceptions;

namespace PowerKraut_Core.vulkan{
    internal static class KrautVK{
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Init(int width, int height, string title, bool fullscreen);

        internal static void InitKrautVK(int width, int height, string title, bool fullscreen){
            var status = Init(width, height, title, fullscreen);

            if (status == 0)
                return;
            
            if (status == -1)
                throw new KrautVKInitFailedException();
            
            if (status == -2)
                throw new KrautVkWindowCreationFailedException();
            
            if (status == -3)
                throw new KrautVKVulkanNotSupportedException();
        }
         
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool WindowShouldClose();
        
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void PollEvents();
        
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Terminate();
    }
}