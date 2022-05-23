import xml2Devs.CodeGen;
import xml2Devs.CoupledModel;
import xmlTools.SrcXMLFileReader;
import xml2Devs.GlobalElements;

public class Main {
    public static void main(String[] args) {
        final String INPUT_FILE_PATH = "./inputXmlFile/Two_Level_SystemCodeGen.xml";
        try {
            SrcXMLFileReader srcxmlFileReader = new SrcXMLFileReader(INPUT_FILE_PATH);
            GlobalElements globalElems = new GlobalElements(srcxmlFileReader.getRoot());
            CodeGen codeGen = new CodeGen(globalElems);
            codeGen.transform();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

    }


}

