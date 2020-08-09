using System;
using System.Collections.Generic;
using System.ComponentModel;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class ColorChange: BaseAction<ColorChange>
    {
        [DefaultValue(0)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "slot")]
        private int _slot;

        [DefaultValue(null)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "primary_colors")]
        private Color[] _primary;
        
        [DefaultValue(null)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "secondary_colors")]
        private Color[] _secondary;

        protected override ColorChange Process(ColorChange action, string username, string from, Dictionary<string, object> parameters)
        {
            if (action._primary == null) action._primary = new []{new Color()};
            if (action._secondary == null) action._secondary = new[] {new Color()};

            foreach (var color in action._primary) ProcessColor(color, parameters);
            foreach (var color in action._secondary) ProcessColor(color, parameters);

            return base.Process(action, username, from, parameters);
        }

        private static void ProcessColor(Color color, Dictionary<string, object> parameters)
        {
            color.Red = StringToInt(color.Red, -2, parameters).ToString();
            color.Green = StringToInt(color.Green, -2, parameters).ToString();
            color.Blue = StringToInt(color.Blue, -2, parameters).ToString();
        }
        
        [Serializable]
        public class Color
        {
            [DefaultValue("-2")]
            [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "red")]
            public string Red = "-2";
        
            [DefaultValue("-2")]
            [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "green")]
            public string Green = "-2";
        
            [DefaultValue("-2")]
            [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "blue")]
            public string Blue = "-2";
        }
    }
}