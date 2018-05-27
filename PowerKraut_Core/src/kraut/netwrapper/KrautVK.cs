/*
Copyright 2018 Jonathan Crockett
 
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

using System.Runtime.InteropServices;
using PowerKraut_Core.kraut.util.exceptions;

namespace PowerKraut_Core.kraut.netwrapper{
    internal static class KrautVK{
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl, EntryPoint = "init")]
        private static extern int Init(int width, int height, string title, bool fullscreen);

        internal static void InitKrautVK(int width, int height, string title, bool fullscreen){
            var status = Init(width, height, title, fullscreen);

            switch (status){
                case 0:
                    return;
                case -1:
                    throw new KrautVKInitFailedException();
                case -2:
                    throw new KrautVkWindowCreationFailedException();
                case -3:
                    throw new KrautVKVulkanNotSupportedException();
                case -4:
                    throw new KrautVKVulkanInstanceCreationFailedException();
                case -5:
                    throw new KrautVKVulkanDeviceCreationFailedException();
                case -6:
                    throw new KrautVKVulkanSurfaceCreationFailedException();
                case -7:
                    throw new KrautVKSemaphoreCreationFailedException();
                default:
                    throw new KrautVKUndefinedException();
            }
        }
         
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl, EntryPoint = "windowShouldClose")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool WindowShouldClose();
        
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl, EntryPoint = "pollEvents")]
        internal static extern void PollEvents();
        
        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl, EntryPoint = "terminate")]
        internal static extern void Terminate();

        [DllImport("lib\\krautvk", CallingConvention = CallingConvention.Cdecl, EntryPoint = "draw")]
        internal static extern void Draw();
    }
}