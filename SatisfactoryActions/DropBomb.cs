using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class DropBomb: BaseAction<DropBomb>
    {

        [DefaultValue("1")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "amount")]
        private string _amount;
        
        [DefaultValue("5")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "time")]
        private string _time;
        
        [DefaultValue("5")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "height")]
        private string _height;

        [DefaultValue("5")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "radius")]
        private string _radius;
        
        [DefaultValue("40")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "damage")]
        private string _damage;
                
        [DefaultValue("5")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "damage_radius")]
        private string _damageRadius;

        
        protected override DropBomb Process(DropBomb action, string username, string from, Dictionary<string, object> parameters)
        {
            action._amount = StringToInt(_amount, 1, parameters).ToString();
            action._time = StringToFloat(_time, 0, parameters).ToString(CultureInfo.InvariantCulture);
            action._height = StringToFloat(_height, 0, parameters).ToString(CultureInfo.InvariantCulture);
            action._radius = StringToFloat(_radius, 0, parameters).ToString(CultureInfo.InvariantCulture);
            action._damage = StringToFloat(_damage, 0, parameters).ToString(CultureInfo.InvariantCulture);
            action._damageRadius = StringToFloat(_damageRadius, 0, parameters).ToString(CultureInfo.InvariantCulture);
            return base.Process(action, username, from, parameters);
        }
    }
}