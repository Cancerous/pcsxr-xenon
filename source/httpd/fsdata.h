static const unsigned char data_index_html[] = {
	/* /index.html */
"<html><body>Prout"
"</body></html>"
};


static struct vfs_entry_s vfs_entries[]={ 
	{"/index.html", data_index_html, sizeof(data_index_html), "text/html; charset=utf-8", 0},
	{0, 0, 0, 0}
};
