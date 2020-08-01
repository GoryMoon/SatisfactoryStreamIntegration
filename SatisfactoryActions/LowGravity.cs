using System.ComponentModel;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class LowGravity: BaseAction<LowGravity>
    {
        [DefaultValue(0.1f)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "amount")]
        private float _amount;
        
        [DefaultValue(300)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "reset_time")]
        private float _resetTime;
    }
}