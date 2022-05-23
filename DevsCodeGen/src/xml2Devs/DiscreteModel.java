package xml2Devs;

import org.dom4j.Element;
import org.dom4j.QName;
import xml2Devs.structs.*;

import java.io.*;
import java.util.*;

import static xmlTools.GeneralParseXMLMethod.findAllChildElementByAttributeText;
import static xmlTools.GeneralParseXMLMethod.*;
public class DiscreteModel {
    private String className;

    private String head_file = "";
    private String cpp_file = "";
    private String constructor = "";
    private String destructor = "";

    private String delta_int = "";
    private String delta_ext = "";
    private String delta_conf = "";
    private String output_func = "";
    private String gc_output = "";
    private String ta = "";
    private String self_defined_functions = "";

    List<ClassFunction> self_defined_funcs;
    List<ClassAttribute> attrs;
    List<Transition> transitions;
    Set<String> stateNames;
    List<SignalEvent> signalEvents;
    Element model;
    GlobalElements globalElements;
    public DiscreteModel(Element model, GlobalElements globalElements) throws IOException {
        this.model = model;
        this.globalElements = globalElements;
        attrs = new ArrayList<>();
        self_defined_funcs = new ArrayList<>();
        transitions = new ArrayList<>();
        stateNames = new HashSet<>();
        className = model.attributeValue("name");
        signalEvents = new ArrayList<>();
        findAttributes();
        findFunctions();
        findSTM();
        add_head_file();
        add_cpp_file();
        writeToFile();
    }
    public void findSTM() {
        Element STM = model.element("ownedBehavior");
        if (STM==null){
            System.out.println("无状态机！图形建模时，离散模型应该建立状态机");
        }
        List <Element> regions = STM.elements("region");
        for (Element region :regions){
            List<Element> states = findChildElementByAttribute(region, "type", "uml:State");
            for (Element state: states){
                if (state.attributeValue("name")!=null){
                    stateNames.add(state.attributeValue("name"));
                    if (state.element("doActivity")!=null){
                        String body = state.element("doActivity").element("body").getText();
                        if (body!=null && !body.equals("")){
                            Transition transition = new Transition();
                            transition.triggerType = "stateAction";
                            transition.srcStateName = state.attributeValue("name");
                            transition.effect = body;
                            transitions.add(transition);
                        }

                    }
                }
                else{
                    System.out.println("无状态名称！图形建模时，需要填写状态名称！");
                }
            }
            List<Element> trans = findChildElementByAttribute(region, "type", "uml:Transition");

            for (Element tran: trans){
                if(tran.element("trigger") != null || tran.attributeValue("guard")!=null ||tran.element("effect")!=null){

                    String eventId = tran.element("trigger").attributeValue("event");
                    Element event = findChildElementByAttribute(globalElements.getModel(), "id", eventId).get(0);
                    if(event.attributeValue("type").equals("uml:TimeEvent")){

                    }else if(event.attributeValue("type").equals("uml:ChangeEvent")){
                        Transition transition = new Transition();
                        String condition = event.element("changeExpression").element("body").getText();
                        if (condition!=null && condition!=""){
                            transition.triggerExpression = condition;
                            transition.triggerType = "ChangeEvent";
                        }
                        if (tran.element("effect")!=null && tran.element("effect").element("body").getText()!=""){
                            transition.effect = tran.element("effect").element("body").getText();
                        }
                        Element srcStateName = findChildElementByAttribute(region, "id", tran.attributeValue("source")).get(0);
                        transition.srcStateName = srcStateName.attributeValue("name");
                        Element tarStateName = findChildElementByAttribute(region, "id", tran.attributeValue("target")).get(0);
                        transition.tarStateName = tarStateName.attributeValue("name");
                        if (tran.attributeValue("guard")!=null){
                            transition.guard = tran.element("ownedRule").element("specification").element("body").getText();
                        }
                        transitions.add(transition);
                    }else if(event.attributeValue("type").equals("uml:SignalEvent")){
                        String id = event.attributeValue("signal");
                        HashMap<String,String> map = globalElements.getTypeMap();
                        List<Element> signalTypes = globalElements.getSignals();
                        for(Element sg : signalTypes) {
                            if (sg.attributeValue("id").equals(id)){
                                if (sg.element("ownedAttribute") != null) {
                                    String type = map.get(sg.element("ownedAttribute").attributeValue(new QName("type")));
                                    List<Connector> connectors = globalElements.getConnectors();
                                    for (Connector conn:connectors){
                                        if (conn.portType.equals(type)){
                                            if (tran.element("effect")!=null && tran.element("effect").element("body").getText()!="") {
                                                String effect = tran.element("effect").element("body").getText();
                                                SignalEvent sig = new SignalEvent();
                                                sig.body = effect;
                                                sig.signalName = conn.portType;
                                                signalEvents.add(sig);
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    public void findFunctions() {
        List<Element> funcList = model.elements("ownedOperation");
        assert funcList != null;
        for (Element func : funcList) {
            ClassFunction classFunction = new ClassFunction();
            classFunction.paraList = new ArrayList<>();
            if (func.attributeValue("name") != null) {
                classFunction.name = func.attributeValue("name");
            }
            List<Element> parameters = func.elements("ownedParameter");
            HashMap<String,String> map = globalElements.getTypeMap();
            for (Element para : parameters) {
                if (para.attributeValue("direction")!=null && para.attributeValue("direction").equals("return")){// 返回值
                    if (map.get(para.attributeValue(new QName("type")))!=null){
                        classFunction.returnType = map.get(para.attributeValue(new QName("type")));
                    }
                    else {
                        List<Element> referenceExtension = findAllChildElementByAttributeText(para, "referentType", "DataType");
                        assert referenceExtension != null;
                        if (referenceExtension.size()>0){
                            String type = referenceExtension.get(0).attributeValue("referentPath");
                            type = type.substring(type.lastIndexOf("::")+2,type.length());
//                            System.out.println(type);
                            classFunction.returnType = type;
                        }
                    }
                }else {// 参数列表
                    ClassAttribute attribute = new ClassAttribute();
                    attribute.name = para.attributeValue("name");
//                    System.out.println( attribute.name);
                    if (map.get(para.attributeValue(new QName("type")))!=null){
                        attribute.type = map.get(para.attributeValue(new QName("type")));
                    }else{
                        System.out.println("参数类型缺失！"+classFunction.name);
                    }
                    // 除了基本属性还有isStatic和readOnly
                    classFunction.paraList.add(attribute);
                }
            }
            if (func.element("method") != null && func.element("method").attributeValue("idref") != null) {
                List<Element> method = findChildElementByAttribute(model, "id", func.element("method").attributeValue("idref"));
                if (method.size()>0){
                    classFunction.body = method.get(0).element("body").getText();
                }
            }
            self_defined_funcs.add(classFunction);
        }
    }
    public void findAttributes() {
        List<Element> ports = findChildElementByAttribute(model, "type", "uml:Port");
        HashMap<String,String> map = globalElements.getTypeMap();
        for (Element tmp : ports) {
            if (map.get(tmp.attributeValue(new QName("type")))!=null) {
//                System.out.println(map.get(tmp.attributeValue(new QName("type"))));
//                System.out.println(tmp.attributeValue("name"));
                ClassAttribute attr = new ClassAttribute();
                attr.name = tmp.attributeValue("name");
                attr.type = "int";
                attr.isStatic = "true";
                attrs.add(attr);
            }
        }
        List<Element> properties = findAllChildElementByAttributeText(model, "type", "uml:Property");

        assert properties != null;
        for (Element tmp : properties) {
            if (map.get(tmp.attributeValue(new QName("type")))!=null){
//                System.out.println(map.get(tmp.attributeValue(new QName("type"))));
//                System.out.println(tmp.attributeValue("name"));
                ClassAttribute attr = new ClassAttribute();
                attr.name = tmp.attributeValue("name");
                attr.type = map.get(tmp.attributeValue(new QName("type")));
                if (tmp.attributeValue("isStatic")!=null){
                    attr.isStatic = tmp.attributeValue("isStatic");
                }
                if (tmp.element("lowerValue")!=null){
                    attr.multiplicity = tmp.element("lowerValue").attributeValue("value");
                }
                attrs.add(attr);
            }

        }

    }
    public Set<String> getStateNames(){
        return stateNames;
    }
    public void add_head_file() {
        head_file += "#pragma once\n";
        head_file += "#ifndef "+className.toUpperCase()+"_H\n";
        head_file += "#define "+className.toUpperCase()+"_H\n";
        head_file += "#include \"ExchangeData.h\"\n";
        head_file += "#include \"Vector.h\"\n";
        head_file += "#include <iostream>\n";
        head_file += "using namespace std;\n";
        head_file += "using namespace adevs;\n";
        head_file += "class "+className+" : public Atomic<IO_Type> {\n";
        head_file += "public:\n\t";
        head_file += className+"(";
        for (ClassFunction func:self_defined_funcs){
            if (func.name.equals(className)){
                for (ClassAttribute attr:func.paraList){
                    head_file += attr.type+" "+attr.name+",";
                }
                head_file = head_file.substring(0,head_file.lastIndexOf(","));
            }
        }
        head_file += ");\n";
        head_file += "\tvoid delta_int();\n";
        head_file += "\tvoid delta_ext(double e, const adevs::Bag<IO_Type>& xb);\n";
        head_file += "\tvoid delta_conf(const adevs::Bag<IO_Type>& xb);\n";
        head_file += "\tvoid output_func(adevs::Bag<IO_Type>& yb);\n";
        head_file += "\tdouble ta();\n";
        head_file += "\tvoid gc_output(adevs::Bag<IO_Type>& g);\n";
        head_file += "\t~"+className+"();\n";
        head_file += "\tconst char* getClassName() { return \""+className.toUpperCase()+"\"; };\n";
        head_file += "\tState getState() {return m_state;}\n";
        head_file += "\tvoid setStateToEnd() {m_state = ENDED;}\n";
        head_file += "private:\n";
        head_file += "\tState              m_state;\n";
        head_file += "\tlist<ExchangeData> outEvents;\n";
        head_file += "public:\n";
        head_file += "\tdouble        elapse;\n\t";
        for (ClassAttribute attr:attrs){
            if (attr.isStatic!=null && attr.isStatic.equals("true")){
                head_file += "static ";
            }
            head_file += attr.type + " " + attr.name;
            if (attr.multiplicity!=null && !attr.multiplicity.equals("")){
                head_file += "[" + attr.multiplicity + "]";
            }
            head_file += ";\n\t";
        }
        head_file += "\n";
        head_file += "public:\n\t";
        for (ClassFunction func:self_defined_funcs){
            if (func.returnType!=null && !func.returnType.equals("")){
                head_file += func.returnType + " " + func.name + "(";
                if (func.paraList.size()>0){
                    for (ClassAttribute attr:func.paraList){
                        head_file += attr.type+" "+attr.name+",";
                    }
                    head_file = head_file.substring(0,head_file.lastIndexOf(","));
                }
                head_file += ");\n\t";
            }
        }
        head_file += "\n";
        head_file += "};\n";
        head_file += "#endif\n";
//        System.out.println(head_file);

    }
    public void add_cpp_file() {
        cpp_file += "#include \""+className+".h\"\n";
        int i=0;
        for (ClassAttribute attr: attrs){// 静态变量初始化
            if (attr.isStatic!=null && attr.isStatic.equals("true")){
                if (attr.multiplicity!=null && !attr.multiplicity.equals("")){
                    if (attr.type.equals("bool")){
                        cpp_file += attr.type + " " + className + "::" + attr.name + "[" + attr.multiplicity + "]={false};\n";
                    }else {// int/double类型的数据初始化
                        cpp_file += attr.type + " " + className + "::" + attr.name + "[" + attr.multiplicity + "]={0};\n";
                    }
                }else {
                    cpp_file += attr.type + " " + className + "::" + attr.name + "=" + i + ";\n";
                    i++;
                }
            }
        }
//        System.out.println(cpp_file);
        cpp_file += add_construct();
        cpp_file += add_delta_int();
        cpp_file += add_output_func();
        cpp_file += add_delta_conf();
        cpp_file += add_delta_ext();
        cpp_file += add_ta();
        cpp_file += add_gc_output();
        cpp_file += add_destructor();
        cpp_file += self_defined_functions();
    }
    public String add_construct() {
        constructor+= className + "::";
        for (ClassFunction func:self_defined_funcs){
            if (func.name.equals(className)){
                constructor += func.name+"(";
                for (ClassAttribute attr:func.paraList){
                    constructor += attr.type+" "+attr.name+",";
                }
                constructor = constructor.substring(0,constructor.lastIndexOf(","));
                constructor += "){\n";
                constructor += func.body + "\n";
                break;
            }
        }
        constructor+="}\n\n";
//        System.out.println(constructor);
        return constructor;
    }

    public String add_delta_int() {
        delta_int+="void " + className + "::delta_int()" + "\n{\n\t";
        for(Transition tran:transitions){
            delta_int+="if (m_state == "+tran.srcStateName;
            if (tran.guard!=null && !tran.guard.equals("")){
                delta_int+="&& "+tran.guard;
            }
            if (tran.triggerExpression!=null && !tran.triggerExpression.equals("")){
                delta_int+=" && "+tran.triggerExpression;
            }
            delta_int+="){\n\t\t";
            if (tran.effect!=null && !tran.effect.equals("")){
                delta_int+=tran.effect + "\n\t";
            }
            if (tran.tarStateName!=null){
                delta_int+="m_state = "+tran.tarStateName+";\n\t";
            }
            delta_int+="}\n\t";
        }
        delta_int+="\n}\n";
        return delta_int;
    }

    public String add_delta_ext() {
        delta_ext += "void " + className + "::" + "delta_ext(double e, const Bag<IO_Type>& xb)" + "\n{\n";
        delta_ext += "\tBag<IO_Type>::const_iterator i = xb.begin();\n";
        delta_ext += "\tfor (; i != xb.end(); i++){\n";
        delta_ext += "\t\tModelType tar = (*i).value->e_target;\n";
        delta_ext += "\t\tModelType src = (*i).value->e_src;\n";
        for (SignalEvent sig:signalEvents){
            delta_ext += "\t\tif ((tar == BROADCAST || tar == ";
            delta_ext += className.toUpperCase()+") && ";
            delta_ext += "src == ";
            List<Connector> conns = globalElements.getConnectors();
            for (Connector conn:conns){
                if (conn.portType.equals(sig.signalName) && conn.tarType.equals(className)){
                    if (conn.src.equals("this")){// 非根复合模型的IBD
                        for (Connector conn2:conns){
                            if (conn2.tarType.equals(conn.srcType)){
                                delta_ext += conn2.srcType.toUpperCase();
                                delta_ext += "){\n";
                                delta_ext += sig.body;
                                delta_ext += "\n\t\t}\n";
                                break;
                            }
                        }
                    }else{
                        List<Element> classes = globalElements.getClassElementList();
                        for (Element clazz:classes){// 跨层IBD
                            if (clazz.attributeValue("name").equals(conn.srcType)){
                                if (isCoupledModel(globalElements.getRoot(),clazz)){
                                    for (Connector conn3:conns) {
                                        if (conn3.tarType.equals(conn.srcType) && conn3.tar.equals("this")) {
                                            delta_ext += conn3.srcType.toUpperCase();
                                            delta_ext += "){\n";
                                            delta_ext += sig.body;
                                            delta_ext += "\n\t\t}\n";
                                            break;
                                        }
                                    }
                                }
                                else{
                                    delta_ext += conn.srcType.toUpperCase();
                                    delta_ext += "){\n";
                                    delta_ext += sig.body;
                                    delta_ext += "\n\t\t}\n";
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
        delta_ext += "\t}\n";
        delta_ext += "}\n";
        return delta_ext;
    }

    public String add_delta_conf() {
        delta_conf+="void " + className + "::" + "delta_conf(const Bag<IO_Type>& xb)" + "\n{\n";
        delta_conf+="\tdelta_int();\n" +
                "\tdelta_ext(0.0, xb);\n";
        delta_conf+="}\n";
        return delta_conf;
    }

    public String add_output_func() {
        output_func+="void " + className + "::" + "output_func(Bag<IO_Type>& yb)" + "\n{\n";
        output_func+="\twhile (!outEvents.empty()) {\n";
        output_func+="\t\tExchangeData data = outEvents.front();\n";
        output_func+="\t\tExchangeData *t = new ExchangeData();\n";
        output_func+="\t\tt->e_req = data.e_req;\n";
        output_func+="\t\tt->e_target = data.e_target;\n";
        output_func+="\t\tt->e_src = data.e_src;\n";
        List<Element> signals = globalElements.getSignals();
        for (Element sig:signals){
            Element prop = sig.element("ownedAttribute");
            if (prop!=null){
                String name = prop.attributeValue("name");
                output_func += "\t\tt->"+ name+" = data."+name+";\n";
            }
        }
        output_func+="\t\tIO_Type y;\n";
        output_func+="\t\ty.port = data.e_port;\n";
        output_func+="\t\ty.value = t;\n";
        output_func+="\t\tyb.insert(y);\n";
        output_func+="\t\toutEvents.pop_front();\n";
        output_func+="\t}\n";
        output_func+="}\n\n";
        return output_func;
    }

    public String add_ta() {
        ta+="double " + className + "::" + "ta()" + "\n{\n";
        ta+= "\tm_curTime = START_TIME + elapse;\n";
        ta+= "\treturn TIME_STEP;\n";
        ta+="}\n\n";
        return ta;
    }

    public String add_gc_output() {
        gc_output+="void " + className + "::" + "gc_output(Bag<IO_Type>& g)" + "\n{\n";
        gc_output+="\tBag<IO_Type>::iterator i;\n" +
                "\tfor (i = g.begin(); i != g.end(); i++)\n" +
                "\t{\n" +
                "\t\tdelete (*i).value;\n" +
                "\t}\n";
        gc_output+="}\n\n";
        return gc_output;
    }

    public String add_destructor() {
        destructor+=className + "::~" + className + "()\n{\n";
        destructor+="}\n\n";
        return destructor;
    }
    public String self_defined_functions() {
        for (ClassFunction func:self_defined_funcs){
            if (!func.name.equals(className) && func.returnType!=null){
                self_defined_functions += func.returnType + " "+ className + "::" + func.name+"(";
                for (ClassAttribute attr:func.paraList){
                    self_defined_functions += attr.type+" "+attr.name+",";
                }
                if (self_defined_functions.lastIndexOf(",")==self_defined_functions.length()-1){
                    self_defined_functions = self_defined_functions.substring(0,self_defined_functions.lastIndexOf(","));
                }
                self_defined_functions += "){\n";
                self_defined_functions += func.body + "\n}\n\n";
            }
        }
        return self_defined_functions;
    }
    public void writeToFile() throws IOException {
        String fileDir = globalElements.getDirPath();
        File head = new File(fileDir+ "/" + className + ".h");
        BufferedWriter fw_head = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(head),"unicode"));
        fw_head.write(head_file.toString());
        fw_head.flush();
        fw_head.close();
        File cpp = new File(fileDir+ "/" +  className + ".cpp");
        BufferedWriter fw_cpp = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(cpp),"unicode"));
        fw_cpp.write(cpp_file.toString());
        fw_cpp.flush();
        fw_cpp.close();
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
}
