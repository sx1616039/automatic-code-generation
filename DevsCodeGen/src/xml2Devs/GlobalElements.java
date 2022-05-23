package xml2Devs;

import org.dom4j.Element;
import org.dom4j.QName;
import xml2Devs.structs.Connector;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import static xmlTools.GeneralParseXMLMethod.*;

public class GlobalElements {
    //  private final String dirPath = "E:\\项目\\桌面文件\\TestGenCode\\TestGenCode";
    private final String dirPath = "./outputFile/";
    private Element model;
    private Element root;
    private List<Element> classElementList;
    private List<Element> structs;
    private List<Element> dataTypes;
    private List<Element> signals;
    private List<Element> constraintBlocks;
    private HashMap<String, String> typeMap;
    private List<Connector> connectors;
    public GlobalElements(Element root) {
        typeMap = new HashMap<>();
        structs = new CopyOnWriteArrayList<Element>();
        classElementList = new CopyOnWriteArrayList<Element>();
        connectors = new ArrayList<>();
        signals = new ArrayList<>();
        constraintBlocks = new ArrayList<>();
        this.root = root;
        init();
    }
    public void init() {
        this.model = findChildElementByName(root, "Model").get(0);
        List<Element> classElements = findAllChildElementByAttributeText(model, "type", "uml:Class");
        assert classElements != null;
        for (Element tmp : classElements) {
            List<Element> constraintBlock = findChildElementByAttribute(root, "base_Class", tmp.attributeValue("id"));
            if(constraintBlock.size() != 0 && constraintBlock.get(0).getName() == "ConstraintBlock"){
                constraintBlocks.add(tmp);
            }else{
                classElementList.add(tmp);
                typeMap.put(tmp.attributeValue("id"), tmp.attributeValue("name"));
            }
        }

        this.dataTypes = findAllChildElementByAttributeText(model, "type", "uml:DataType");
        assert dataTypes != null;
        for (Element tmp : dataTypes) {
//            System.out.println(tmp.attributeValue("id")+tmp.attributeValue("name"));
            typeMap.put(tmp.attributeValue("id"), tmp.attributeValue("name"));
        }
        for (Element tmp : dataTypes) {
            List<Element> properties = findAllChildElementByAttributeText(tmp, "type", "uml:Property");
            assert properties != null;
            if (!properties.isEmpty()){
                structs.add(tmp);
//                System.out.println(tmp.attributeValue("id")+tmp.attributeValue("name"));
            }
        }
        List<Element> signalTypes = findAllChildElementByAttributeText(root, "type", "uml:Signal");
        assert signalTypes != null;
        signals.addAll(signalTypes);
    }
    public String projectName() {
        return model.attributeValue("name");
    }
    public List<Connector> getConnectors(){
        return connectors;
    }
    public List<Element> getSignals(){
        return signals;
    }
    public void addConnectors(List<Connector> connectors) {
        this.connectors.addAll(connectors);
    }

    public Element getModel() {
        return model;
    }
    public List<Element> getConstraintBlocks(){
        return constraintBlocks;
    }
    public void setModel(Element model) {
        this.model = model;
    }
    public String getDirPath(){
        return dirPath;
    }
    public Element getRoot() {
        return root;
    }

    public void setRoot(Element root) {
        this.root = root;
    }

    public List<Element> getClassElementList() {
        return classElementList;
    }

    public void setClassElementList(List<Element> classElementList) {
        this.classElementList = classElementList;
    }

    public List<Element> getStructs() {
        return structs;
    }

    public void setStructs(List<Element> structs) {
        this.structs = structs;
    }
    public List<Element> getDataTypes() {
        return dataTypes;
    }

    public void setDataTypes(List<Element> dataTypes) {
        this.dataTypes = dataTypes;
    }

    public HashMap<String, String> getTypeMap() {
        return typeMap;
    }

    public void setTypeMap(HashMap<String, String> typeMap) {
        this.typeMap = typeMap;
    }

}
