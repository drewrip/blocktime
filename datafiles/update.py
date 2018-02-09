from apiclient import discovery, http
from httplib2 import Http
from oauth2client import file, client, tools
import sys
from time import gmtime, strftime

SCOPES = (
	'https://www.googleapis.com/auth/drive',
	'https://www.googleapis.com/auth/drive.appdata',
	'https://www.googleapis.com/auth/drive.file',
	'https://www.googleapis.com/auth/drive.readonly.metadata'
)

store = file.Storage('storage.json')
creds = store.get()
if not creds or creds.invalid:
    flow = client.flow_from_clientsecrets('client_id.json', SCOPES)
    creds = tools.run_flow(flow, store)
drive = discovery.build('drive', 'v3', http=creds.authorize(Http()))

files = drive.files().list().execute().get('files', [])
for i in files:
	if(i["name"] == "BlockTimeFiles"):
		dirId = i["id"]
		break

file_metadata = {'name': strftime("%m-%d %H:%M")+'.csv', "parents":[dirId]}
media = http.MediaFileUpload('timedata.csv',mimetype='text/csv')
file = drive.files().create(body=file_metadata,media_body=media,fields='id')

file.execute()
