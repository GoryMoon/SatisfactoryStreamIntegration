using System.ComponentModel;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class Emote: BaseAction<Emote>
    {
        [DefaultValue("Clap")]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "style")]
        private string _style;
    }
}