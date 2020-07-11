using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class TriggerFuse: BaseAction<TriggerFuse>
    {
        [DefaultValue(100f)]
        [JsonProperty(PropertyName = "chance", DefaultValueHandling = DefaultValueHandling.Populate)]
        private string _chance;

        protected override TriggerFuse Process(TriggerFuse action, string username, string from, Dictionary<string, object> parameters)
        {
            action._chance = StringToFloat(_chance, 0, parameters).ToString(CultureInfo.InvariantCulture);
            return base.Process(action, username, from, parameters);
        }
    }
}