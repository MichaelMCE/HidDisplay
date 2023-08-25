function HttpGetWaitSpawnable($url) {
	HttpGetWait($url);
}

function SpawnHttpRequest($url) {
	SpawnThread("HttpGetWaitSpawnable",, list($url));
}
