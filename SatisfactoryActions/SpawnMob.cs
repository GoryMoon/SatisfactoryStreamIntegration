using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
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
        
        [DefaultValue(true)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "persistent")]
        private bool _persistent;
        
        [DefaultValue("1")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "scale_min")]
        private string _scaleMin;
        
        [DefaultValue("1")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "scale_max")]
        private string _scaleMax;
        
        protected override SpawnMob Process(SpawnMob action, string username, string from, Dictionary<string, object> parameters)
        {
            action._amount = StringToInt(_amount, 1, parameters).ToString();
            action._scaleMin = StringToFloat(_scaleMin, 0.1f, parameters).ToString(CultureInfo.InvariantCulture);
            action._scaleMax = StringToFloat(_scaleMax, 0.1f, parameters).ToString(CultureInfo.InvariantCulture);
            return base.Process(action, username, from, parameters);
        }
    }
}