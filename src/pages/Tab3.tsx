import { IonContent, IonHeader, IonPage, IonTitle, IonToolbar } from '@ionic/react';
import './Tab3.css';
import { DarkButton, Input, InputWithSeparatorContainer, Label } from 'pokemon-studio-components';
import { RubyVM } from '@root/plugins/ruby-vm/dist/esm';

const Tab3: React.FC = () => {

  const callPlugin = async () => {
    const { interpreter } = await RubyVM.create({
      executionLocation: "",
      archiveLocation: ""
    });
    const result = await RubyVM.execute({
       interpreter
    })
    console.log("finished");
    return result;
  };

  return (
    <IonPage>
      <IonHeader>
        <IonToolbar>
          <IonTitle>Tab 3</IonTitle>
        </IonToolbar>
      </IonHeader>
      <IonContent fullscreen>
        <IonHeader collapse="condense">
          <IonToolbar>
            <IonTitle size="large">Tab 3</IonTitle>
          </IonToolbar>
        </IonHeader>

      <InputWithSeparatorContainer>
        <Label>Coucou</Label>
        <Input name="projectPath" />
      </InputWithSeparatorContainer>
      <DarkButton onClick={callPlugin}>Click</DarkButton>
      
      </IonContent>
    </IonPage>
  );
};

export default Tab3;
