package xmlTools;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;

import java.io.File;

public class SrcXMLFileReader {
    private Element root;
    private Document document;
    private String fileName;


    public SrcXMLFileReader(String fileName) throws DocumentException {
        this.document = new SAXReader().read(new File(fileName));
        this.root = document.getRootElement();
        this.fileName = fileName;
    }

    public Element getRoot() {
        return root;
    }

    public void setRoot(Element root) {
        this.root = root;
    }

    public Document getDocument() {
        return document;
    }

    public void setDocument(Document document) {
        this.document = document;
    }
}


