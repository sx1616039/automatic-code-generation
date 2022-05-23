package xml2Devs;

import org.dom4j.Element;
import org.dom4j.QName;
import static xmlTools.GeneralParseXMLMethod.*;

import java.io.*;
import java.util.*;

public class CodeGen {
    private GlobalElements globalElements;
    private List<DiscreteModel> discreteModels;
    private List<ContinuousModel> continuousModels;
    private List<CoupledModel> coupledModels;
    Set<String> stateNames;
    public CodeGen(GlobalElements globalElements) throws IOException {
        this.globalElements = globalElements;
        discreteModels = new ArrayList<DiscreteModel>();
        continuousModels = new ArrayList<ContinuousModel>();
        coupledModels = new ArrayList<CoupledModel>();
        stateNames = new HashSet<>();
    }

    private void createExchangeData() throws IOException {
        String head_file="";
        String cpp_file = "";
        String fileName = "ExchangeData";
        head_file += "#pragma once\n";
        head_file += "#ifndef "+fileName.toUpperCase()+"_H\n";
        head_file += "#define "+fileName.toUpperCase()+"_H\n";
        head_file += "#include \"adevs\\include\\adevs.h\"\n";
        head_file += "#include \"Vector.h\"\n";
        head_file += "#include \"fstream\"\n";
        List<Element> structs = globalElements.getStructs();
        HashMap<String,String> map = globalElements.getTypeMap();
        for (Element struct: structs){
            if (struct.attributeValue("name").equals("Constant")){
                List<Element> properties = findChildElementByAttribute(struct, "type", "uml:Property");
                for (Element prop:properties){
                    String name = prop.attributeValue("name");
                    String type = map.get(prop.attributeValue(new QName("type")));
                    String value = prop.element("defaultValue").attributeValue("value");
                    head_file += "const "+ type + " "+ name+" = "+value+";\n";
                }
            }
        }
        for (ContinuousModel continuousModel:continuousModels){
            stateNames.addAll(continuousModel.getStateNames());
        }
        for (DiscreteModel discreteModel: discreteModels){
            stateNames.addAll(discreteModel.getStateNames());
        }
        head_file += "enum State{ ";
        for (String name: stateNames){
            head_file += name + ",";
        }
        head_file += "ENDED";
        head_file += " };\n";
        head_file += "enum ModelType{ ";
        for (Element clazz: globalElements.getClassElementList()){
            head_file += clazz.attributeValue("name").toUpperCase() + ",";
        }
        head_file += "BROADCAST";
        head_file += " };\n";
        for (Element struct: structs) {
            if (!struct.attributeValue("name").equals("Constant")) {
                head_file += "typedef struct "+ struct.attributeValue("name")+" {\n\t";
                List<Element> properties = findChildElementByAttribute(struct, "type", "uml:Property");
                for (Element prop:properties){
                    String name = prop.attributeValue("name");
                    String type = map.get(prop.attributeValue(new QName("type")));
                    head_file += type + " "+ name+";\n\t";
                }
                head_file += "\n}"+ struct.attributeValue("name")+";\n";
            }
        }
        head_file += "struct ExchangeData{\n\t";
        head_file += "int           e_req;\n\t";
        head_file += "int           e_port;\n\t";
        head_file += "ModelType     e_target;\n\t";
        head_file += "ModelType     e_src;\n\t";
        List<Element> signals = globalElements.getSignals();
        for (Element sig:signals){
            Element prop = sig.element("ownedAttribute");
            if (prop!=null){
                String name = prop.attributeValue("name");
                String type = map.get(prop.attributeValue(new QName("type")));
                head_file += type + " "+ name+";\n\t";
            }
        }
        head_file += "\n";
        head_file += " };\n";
        head_file += "typedef adevs::PortValue<ExchangeData*> IO_Type;\n";
        head_file += "#endif\n";
        writeToFile(fileName,head_file,cpp_file);
    }

    public void transform() throws IOException {
        // 解析复合模型
        for (Element model : globalElements.getClassElementList()) {
            if (isCoupledModel(globalElements.getRoot(), model)) {
                CoupledModel coupledModel = new CoupledModel(model, globalElements);
                coupledModels.add(coupledModel);
            }
        }
        // 必须先解析复合模型，再解析原子模型
        for (Element model : globalElements.getClassElementList()) {
            if (!isCoupledModel(globalElements.getRoot(), model) && isAtomicModel(globalElements.getRoot(),model)){
                if (isContinuousModel(globalElements.getRoot(),model)){
                    ContinuousModel continuousModel = new ContinuousModel(model,globalElements);
                    continuousModels.add(continuousModel);
                }
                else {
                    DiscreteModel discreteModel = new DiscreteModel(model,globalElements);
                    discreteModels.add(discreteModel);
                }
            }
        }
        createListener();
        createMain();
        createExchangeData();
    }
    public void createListener() throws IOException {
        String head_file="";
        String cpp_file = "";
        String fileName = "Listener";
        head_file += "#pragma once\n";
        head_file += "#ifndef "+fileName.toUpperCase()+"_H\n";
        head_file += "#define "+fileName.toUpperCase()+"_H\n";
        head_file += "#include \"ExchangeData.h\"\n";
        for (Element model : globalElements.getClassElementList()) {
            if (!isCoupledModel(globalElements.getRoot(), model) && isAtomicModel(globalElements.getRoot(),model)) {
                String modelName = model.attributeValue("name");
                head_file += "#include \""+modelName+".h\"\n";
            }
        }
        head_file += "class "+ fileName+": public EventListener<IO_Type>{\n";
        head_file += "public:\n\t";
        head_file += fileName+"();\n\t";
        head_file += "~"+fileName+"();\n\t";
        head_file += "void outputEvent(Event<IO_Type> y, double t);\n\t";
        head_file += "void stateChange(Atomic<IO_Type>* model, double t);\n";
        head_file += "public:\n\t";
        head_file += "int cnt;\n";
        head_file += "};\n";
        head_file += "#endif\n";

        cpp_file+= "#include \"Listener.h\"\n";
        cpp_file+= "Listener::Listener(){\n\t";
        cpp_file+= "cnt = 0;\n}\n";
        cpp_file+= "Listener::~Listener(){\n";
        cpp_file+= "}\n";
        cpp_file+= "void Listener::stateChange(Atomic<IO_Type>* model, double t){\n";
        for (Element model : globalElements.getClassElementList()) {
            if (!isCoupledModel(globalElements.getRoot(), model) && isAtomicModel(globalElements.getRoot(),model)) {
                String modelName = model.attributeValue("name");
                cpp_file += "\t";
                if (isContinuousModel(globalElements.getRoot(),model)){
                    cpp_file += "if(model->getClassName() == \"Hybrid\") {\n\t\t";
                    cpp_file += "Hybrid<IO_Type> *h = (Hybrid<IO_Type>*)model;\n\t\t";
                    cpp_file += "ode_system<IO_Type> *a = h->getSystem();\n\t\t";
                    cpp_file += "if (a->getClassName() == \""+modelName.toUpperCase()+"\") {\n\t\t\t";
                    cpp_file += modelName+" *b = ("+modelName+"*)a;\n\t\t\t";
                    cpp_file += "b->elapse = t;\n\t\t";
                    cpp_file += "}\n\t";
                    cpp_file += "}\n";
                }else{
                    cpp_file += "if (model->getClassName() == \""+modelName.toUpperCase()+"\") {\n\t\t";
                    cpp_file += modelName+" *a = ("+modelName + "*)model;\n\t\t";
                    cpp_file += "a->elapse = t;\n\t";
                    cpp_file += "}\n";
                }
            }
        }
        cpp_file+= "}\n";
        cpp_file+= "void Listener::outputEvent(Event<IO_Type> y, double t){\n";
        cpp_file+= "}\n";
        writeToFile(fileName,head_file,cpp_file);
    }
    public void createMain() throws IOException {
        String head_file="";
        String cpp_file = "";
        String fileName = "main";
        cpp_file += "#include \"DefendSystem.h\"\n";
        cpp_file += "#include \"ReadInitFile.h\"\n";
        cpp_file += "#include \"Vector.h\"\n";
        cpp_file += "#include \"ExchangeData.h\"\n";
        cpp_file += "#include \"Listener.h\"\n";
        cpp_file += "#include <iostream>\n";
        cpp_file += "using namespace std;\n";
        cpp_file += "int main(int argc, const char*argv[]){\n\t";
        cpp_file += "char fileName[] = \"initFile.xml\";\n\t";
        cpp_file += "ReadInitFile *read = new ReadInitFile(fileName);\n\t";
        cpp_file += "DefendSystem *system = new DefendSystem(read->m_misParameters,read->m_tarParameters);\n\t";
        cpp_file += "adevs::Simulator<IO_Type> sim(system);\n\t";
        cpp_file += "Listener* listener = new Listener();\n\t";
        cpp_file += "sim.addEventListener(listener);\n\t";
        cpp_file += "sim.execUntil(2000);\n\t";

//        cpp_file += "while (true){\n\t\t";
//        cpp_file += "if (listener->cnt >= read->m_tarParameters.size()) {\n\t\t\t";
//        cpp_file += "break;\n\t\t";
//        cpp_file += "}\n\t";
//        cpp_file += "}\n\t";
//        cpp_file += "delete system;\n\t";
        cpp_file += "return 0;\n";
        cpp_file += "}\n";
        writeToFile(fileName,"",cpp_file);
    }

    //判断是否是复合模型，只要有一个property是一个Block，那他就是复合模型
    private boolean isCoupledModel(Element root, Element element){
        List<Element> properties = findAllChildElementByAttributeText(element, "type", "uml:Property");
        for(Element property : properties){
            if(property.attributeValue(new QName("type")) != null){
                List<Element> bC = findChildElementByAttribute(root, "base_Class", property.attributeValue(new QName("type")));
                if(bC.size() != 0 && bC.get(0).getName() == "Block"){
                    return true;
                }
            }
        }
        return false;
    }
    //判断一个Class是不是一个block
    private boolean isAtomicModel(Element root, Element element){
        if(findChildElementByAttribute(root, "base_Class", element.attributeValue("id")).size() == 0){
            return false;
        }
        if(findChildElementByAttribute(root, "base_Class", element.attributeValue("id")).get(0).getName() == "Block"){
            return true;
        }else {
            return false;
        }
    }
    //判断是否是连续模型，只要有一个ConstraintBlock property，那他就是连续模型
    private boolean isContinuousModel(Element root, Element element){
        List<Element> properties = findAllChildElementByAttributeText(element, "type", "uml:Property");
        for(Element property : properties){
            if(property.attributeValue(new QName("type")) != null){
                List<Element> bC = findChildElementByAttribute(root, "base_Class", property.attributeValue(new QName("type")));
                if(bC.size() != 0 && bC.get(0).getName() == "ConstraintBlock"){
                    return true;
                }
            }
        }
        return false;
    }
    public void writeToFile(String fileName, String head_file,String cpp_file) throws IOException {
        String fileDir = globalElements.getDirPath();
        if (!head_file.equals("")){
            File head = new File(fileDir+ "/"  + fileName + ".h");
            BufferedWriter fw_head = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(head),"unicode"));
            fw_head.write(head_file.toString());
            fw_head.flush();
            fw_head.close();
        }
        if (!cpp_file.equals("")){
            File cpp = new File(fileDir+ "/" + fileName + ".cpp");
            BufferedWriter fw_cpp = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(cpp),"unicode"));
            fw_cpp.write(cpp_file.toString());
            fw_cpp.flush();
            fw_cpp.close();
        }
    }
}
