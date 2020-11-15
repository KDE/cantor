function get_files
{
    echo cantor.xml
}

function po_for_file
{
    case "$1" in
       cantor.xml)
           echo cantor_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       cantor.xml)
           echo comment
       ;;
    esac
}
