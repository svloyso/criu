POSTDUMP="post-dump"
PRERESTORE="pre-restore"
DUMPARGS="--create --absolute-names --gzip --no-unquote --no-wildcards --file"
RESTOREARGS="--extract --gzip --no-unquote --no-wildcards --absolute-names --directory / --file"
IMGFILE="tmpfiles.tar.gz"

case "$CRTOOLS_SCRIPT_ACTION" in
	$POSTDUMP )
		if [ "$#" -lt 1 ]; then
			echo "savefiles.sh: ERROR! No files is given."
			exit 1
		fi
		tar $DUMPARGS $IMGFILE -- "$@"
		exit $?
		;;
	$PRERESTORE )
		if [ "$#" -ne 0 ]; then 
			echo "savefiles.sh: ERROR! Not expected script args."
			exit 1
		fi
		tar $RESTOREARGS $IMGFILE
		exit $?
		;;
esac

exit 0
