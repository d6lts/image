CREATE TABLE image (
  nid integer not null default '0',
  image_path varchar(255) not null default '',
  thumb_path varchar(255) not null default '',
  preview_path varchar(255) not null default '',
  format varchar(255) not null default '',
  width integer not null default '0',
  height integer not null default '0',
  filesize integer not null default '0',
  iptc text not null default '',
  exif text not null default '',
  personal smallint not null default '0',
  weight smallint not null default '0'
}
