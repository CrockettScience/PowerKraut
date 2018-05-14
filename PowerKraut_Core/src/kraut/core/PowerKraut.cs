using System;
using System.ComponentModel.DataAnnotations;
using static PowerKraut_Core.vulkan.KrautVK;

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
            Init(width, height, gameTitle, fullscreen);

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
            PkInstance.Start(800, 600, "KrautDemo", false);
        }
    }
}
