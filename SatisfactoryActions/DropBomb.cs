using System.Collections.Generic;
using System.ComponentModel;
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
            action._time = StringToInt(_time, 0, parameters).ToString();
            action._height = StringToInt(_height, 0, parameters).ToString();
            action._radius = StringToInt(_radius, 0, parameters).ToString();
            action._damage = StringToInt(_damage, 0, parameters).ToString();
            action._damageRadius = StringToInt(_damageRadius, 0, parameters).ToString();
            return base.Process(action, username, from, parameters);
        }
    }
}