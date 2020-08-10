using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
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
            color.Red = StringToFloat(color.Red, -3, parameters).ToString(CultureInfo.InvariantCulture);
            color.Green = StringToFloat(color.Green, -3, parameters).ToString(CultureInfo.InvariantCulture);
            color.Blue = StringToFloat(color.Blue, -3, parameters).ToString(CultureInfo.InvariantCulture);
        }
        
        [Serializable]
        public class Color
        {
            [DefaultValue("-3")]
            [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "red")]
            public string Red = "-3";
        
            [DefaultValue("-3")]
            [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "green")]
            public string Green = "-3";
        
            [DefaultValue("-3")]
            [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "blue")]
            public string Blue = "-3";
        }
    }
}