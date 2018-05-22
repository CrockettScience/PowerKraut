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

using System;
using System.ComponentModel.DataAnnotations;
using System.Diagnostics;
using System.Reflection;
using static PowerKraut_Core.kraut.netwrapper.KrautVK;

namespace PowerKraut_Core.kraut.core{
    /// <summary>
    /// This is the entry point for the engine in source.
    /// </summary>
    public sealed class PowerKrautInstance{

        private static PowerKrautInstance _pkSingleton;
        
        /// <summary>
        /// Holds the current active instance of PowerKraut
        /// </summary>
        public static  PowerKrautInstance PkInstance => _pkSingleton ?? (_pkSingleton = new PowerKrautInstance());

        /// <summary>
        /// Initializes Vulkan, opens a window and loads the first scene
        /// </summary>
        public void Start(int width, int height, string gameTitle, bool fullscreen){
            #if DEBUG
                Console.WriteLine("PowerKraut Debug\nPID: " + Process.GetCurrentProcess().Id);
            #endif
            
            
            InitKrautVK(width, height, gameTitle, fullscreen);

            try{
                Loop();
            }
            
            catch (Exception e){
                Console.WriteLine(e);
                throw;
            }
            
            finally{
                Terminate();
            }
        }

        private void Loop(){
            while (!WindowShouldClose()){
                PollEvents();
            }
        }

        private static void Main(string[] args){
            
            PkInstance.Start(2160, 1024, "KrautDemo", false);
        }
    }
}
