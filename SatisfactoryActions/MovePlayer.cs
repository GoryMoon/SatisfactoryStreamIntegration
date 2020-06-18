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
        
        protected override MovePlayer Process(MovePlayer action, string username, string from, Dictionary<string, object> parameters)
        {
            action._amount = StringToDouble(_amount, 0, parameters).ToString(CultureInfo.InvariantCulture);
            return base.Process(action, username, from, parameters);
        }
    }
}