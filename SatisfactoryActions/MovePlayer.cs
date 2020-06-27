using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class MovePlayer: BaseAction<MovePlayer>
    {
        [DefaultValue("10")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "amount")]
        private string _amount;
        
        [DefaultValue("-1")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "amount_vertical")]
        private string _amountVertical;
        
        [DefaultValue(5.0f)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "no_fall_damage")]
        private float _noFallDamage;
        
        protected override MovePlayer Process(MovePlayer action, string username, string from, Dictionary<string, object> parameters)
        {
            action._amount = StringToDouble(_amount, 1, parameters).ToString(CultureInfo.InvariantCulture);
            action._amountVertical = StringToDouble(_amountVertical, -1, parameters).ToString(CultureInfo.InvariantCulture);
            return base.Process(action, username, from, parameters);
        }
    }
}