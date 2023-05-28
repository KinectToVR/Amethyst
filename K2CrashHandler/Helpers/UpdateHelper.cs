using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Data.Json;
using Windows.Web.Http;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;
using Microsoft.VisualBasic;
using Windows.Foundation.Collections;
using Windows.Management.Deployment;

// ReSharper disable ClassNeverInstantiated.Global
// ReSharper disable InconsistentNaming

namespace K2CrashHandler.Helpers;

public static class UpdateHelper
{
    // AppCenter tokens
    private const string AppSecret = "AZ_APPCENTER_SECRET";
    private const string ApiToken = "AZ_APPCENTER_RO_TOKEN";

    public static async Task CheckForUpdates()
    {
        try
        {
            using var client = new HttpClient();

            client.DefaultRequestHeaders.Add(new KeyValuePair<string, string>("X-API-Token", ApiToken));
            using var response =
                await client.GetAsync(new Uri($"https://api.appcenter.ms/v0.1/sdk/apps/{AppSecret}/releases/latest"));

            using var content = response.Content;
            var json = await content.ReadAsStringAsync();

            // Deserialize as the prepared object class, compare
            var release = JsonConvert.DeserializeObject<AppRelease>(json);
            var updateFound = release?.version.CompareTo(Package.Current.Id.Version.AsVersion()) > 0;

            // If there's an update, try to download and install it
            if (release?.status is "available" && updateFound)
                await new PackageManager().UpdatePackageAsync(new Uri(release.download_url),
                    null, DeploymentOptions.ForceApplicationShutdown);
        }
        catch (Exception)
        {
            // ignored
        }
    }

    internal class AppRelease
    {
        public string app_name { get; set; }
        public string app_display_name { get; set; }
        public string app_os { get; set; }
        public string app_icon_url { get; set; }
        public string release_notes_url { get; set; }
        public Owner owner { get; set; }
        public bool is_external_build { get; set; }
        public string origin { get; set; }
        public int id { get; set; }
        public Version version { get; set; }
        public string short_version { get; set; }
        public int size { get; set; }
        public string min_os { get; set; }
        public object device_family { get; set; }
        public string bundle_identifier { get; set; }
        public string fingerprint { get; set; }
        public DateTime uploaded_at { get; set; }
        public string download_url { get; set; }
        public string install_url { get; set; }
        public bool mandatory_update { get; set; }
        public bool enabled { get; set; }
        public string fileExtension { get; set; }
        public bool is_latest { get; set; }
        public string release_notes { get; set; }
        public List<string> package_hashes { get; set; }
        public string destination_type { get; set; }
        public string status { get; set; }
        public string distribution_group_id { get; set; }
        public List<DistributionGroups> distribution_groups { get; set; }

        internal class Owner
        {
            public string name { get; set; }
            public string display_name { get; set; }
        }

        public class DistributionGroups
        {
            public string id { get; set; }
            public string name { get; set; }
            public string origin { get; set; }
            public string display_name { get; set; }
            public bool is_public { get; set; }
        }
    }
}

public static class VersionExtensions
{
    public static Version AsVersion(this PackageVersion version)
    {
        return new Version(version.Major, version.Minor, version.Build, version.Revision);
    }
}