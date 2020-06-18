using System.Collections.Generic;
using System.ComponentModel;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class SpawnMob: BaseAction<SpawnMob>
    {
        [DefaultValue("SpaceRabbit")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "id")]
        private string _id;
        
        [DefaultValue("1")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "amount")]
        private string _amount;
        
        [DefaultValue(10)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "radius")]
        private double _radius;
        
        protected override SpawnMob Process(SpawnMob action, string username, string from, Dictionary<string, object> parameters)
        {
            action._amount = StringToInt(_amount, 0, parameters).ToString();
            return base.Process(action, username, from, parameters);
        }
    }
}