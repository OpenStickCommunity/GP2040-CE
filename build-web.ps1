$website_dir = "www/build/"
$fsdata_filename = "lib/httpd/fsdata.c"

Write-Host "Building React App"
Push-Location
Set-Location www

if (-Not ( Test-Path node_modules )) {
    Write-Host "Installing NPM dependencies"
    npm install
}
Write-Host "Running NPM Build"

[Environment]::SetEnvironmentVariable( "NODE_OPTIONS", "--openssl-legacy-provider")

npm run build

Pop-Location

Write-Host "React App Built"

Write-Host "Generating $fsdata_filename"

./tools/makefsdata $website_dir -f:$fsdata_filename

$fsData = Get-Content $fsdata_filename

Set-Content -Path $fsdata_filename -Value ($fsData -replace '#include "lwip/apps/fs.h"', '#include "fsdata.h"' -replace '#include "lwip/def.h"')

