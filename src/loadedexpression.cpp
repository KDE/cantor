/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "loadedexpression.h"

#include "lib/imageresult.h"
#include "lib/epsresult.h"
#include "lib/textresult.h"

#include <kglobal.h>
#include <kstandarddirs.h>

LoadedExpression::LoadedExpression( MathematiK::Session* session ) : MathematiK::Expression( session )
{

}

LoadedExpression::~LoadedExpression()
{

}

void LoadedExpression::interrupt()
{
    //Do nothing
}

void LoadedExpression::evaluate()
{
    //Do nothing
}

void LoadedExpression::loadFromXml(const QDomElement& xml, const KZip& file)
{
    setCommand(xml.firstChildElement("Command").text());

    QDomElement resultElement=xml.firstChildElement("Result");
    MathematiK::Result* result=0;
    if (resultElement.attribute("type") == "text")
    {
        result=new MathematiK::TextResult(resultElement.text());
    }
    else if (resultElement.attribute("type") == "image" )
    {
        const KArchiveEntry* imageEntry=file.directory()->entry(resultElement.attribute("filename"));
        if (imageEntry&&imageEntry->isFile())
        {
            const KArchiveFile* imageFile=static_cast<const KArchiveFile*>(imageEntry);
            QString dir=KGlobal::dirs()->saveLocation("tmp", "mathematik/");
            imageFile->copyTo(dir);
            KUrl imageUrl=dir+'/'+imageFile->name();
            if(imageFile->name().endsWith(QLatin1String(".eps")))
                result=new MathematiK::EpsResult(imageUrl);
            else
                result=new MathematiK::ImageResult(imageUrl);
        }
    }

    setResult(result);
}
